/**
 * @file
 * @brief Implementación del protocolo de comunicación
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andrés Martinez Varón <jorgeandre@unicauca.edu.co>
 * @copyright MIT License
 */
#include "protocol.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "versions.h"

/**
 * @brief Manda un archivo al socket de destino
 *
 * @param s socket de destino
 * @param filename nombre del archivo local que se enviará
 * @return int 0 en caso de exito, -1 en caso de error
 */
int send_file(int s, char *filename);

/**
 * @brief Recibe un archivo del socket
 * @param s socket del que se recibe el archivo
 * @param filename nombre del archivo local que se recibirá
 * @return int 0 en caso de exito, -1 en caso de error
 */
int receive_file(int s, char *filename);

/**
 * @brief Envia las versiones del repositorio al cliente
 *
 * @param s socket del cliente
 * @param filename nombre del archivo
 * @return int 0 para exito, -1 para error
 */
int send_versions(int s, char *filename);

/**
 * @brief Ejecuta el protocolo del servidor en el método add
 *
 * @param s socket del cliente
 * @return sres_code respuesta mandada
 */
int server_add(int s);

/**
 * @brief Ejecuta el protocolo del servidor en el método get
 *
 * @param s socket del cliente
 * @return sres_code respuesta mandada
 */
int server_get(int s);

/**
 * @brief Ejecuta el protocolo del servidor en el método list
 *
 * @param s socket del cliente
 * @return sres_code respuesta mandada
 */
int server_list(int s);

int send_greeting(int s, const int greeter) {
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);
    if (greeter) {
        strcpy(buf, "REMOTE");
    } else {
        strcpy(buf, "VERSIONS");
    }

    if (write(s, buf, BUFSZ) == -1) {
        return -1;
    }
    return 0;
}

int receive_greeting(int s, const int greeter) {
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);
    if (read(s, buf, BUFSZ) == -1) {
        return -1;
    }
    // Valida la respuesta correcta
    if (greeter && strcmp(buf, "VERSIONS")) {
        return -1;
    }
    if (!greeter && strcmp(buf, "REMOTE")) {
        strcpy(buf, "DENY");
        write(s, buf, BUFSZ);
        return -1;
    }
    puts(buf);
    return 0;
}

sres_code client_add(int s, char *filename, char *comment) {
    const method_code method = ADD;
    struct add_request request;
    char hash[HASH_SIZE];
    sres_code rserver;
    struct stat file_stat;

    // 1. Enviar el método
    if (write(s, &method, sizeof(method_code)) == -1) {
        return RSOCKET_ERROR;
    }

    // 2. Enviar el nombre del archivo, hash y comentario
    get_file_hash(filename, hash);

    memset(&request, 0, sizeof(request));
    strcpy(request.filename, filename);
    strcpy(request.comment, comment);
    strcpy(request.hash, hash);

    if (write(s, &request, sizeof(request)) == -1) {
        return RSOCKET_ERROR;
    }

    // 3. Recibir una respuesta del servidor (indica si se puede subir el
    // archivo)
    if (read(s, &rserver, sizeof(sres_code)) == -1) {
        return RSOCKET_ERROR;
    }
    if (rserver != RSERVER_OK) {
        return rserver;
    }

    // 4. Manda el archivo
    if (send_file(s, filename) == -1) {
        return RERROR;
    }

    // 5. Recibe la respuesta del servidor
    if (read(s, &rserver, sizeof(sres_code)) == -1) {
        return RSOCKET_ERROR;
    }

    if (rserver != RSERVER_OK) {
        return rserver;
    }

    return RSERVER_OK;
}

sres_code client_get(int s, char *filename, int version) {
    const method_code method = GET;
    struct get_request request;
    sres_code rserver;
    cres_code cres;
    char server_hash[HASH_SIZE];
    char local_hash[HASH_SIZE];

    // 1. Enviar el método
    if (write(s, &method, sizeof(method_code)) == -1) {
        return RSOCKET_ERROR;
    }

    // 2. Enviar el nombre del archivo y la versión
    memset(&request, 0, sizeof(request));
    request.version = version;
    strcpy(request.filename, filename);
    if (write(s, &request, sizeof(request)) == -1) {
        return RSOCKET_ERROR;
    }

    // 3. Recibe la respuesta del servidor
    if (read(s, &rserver, sizeof(sres_code)) == -1) {
        return RSOCKET_ERROR;
    }
    if (rserver != RSERVER_OK) {
        return rserver;
    }

    // 4. Recibe el hash de la versión
    if (read(s, &server_hash, HASH_SIZE) == -1) {
        return RSOCKET_ERROR;
    }

    // 5. responde si el archivo local ya esta actualizado
    if (get_file_hash(filename, local_hash) == 0) {
        return RERROR;
    }
    if (EQUALS(server_hash, local_hash)) {
        cres = DENY;
    } else {
        cres = CONFIRM;
    }
    if (write(s, &cres, sizeof(cres_code)) == -1) {
        return RSOCKET_ERROR;
    }

    // 6. Recibe el archivo
    rserver = receive_file(s, filename);
    if (rserver != RSERVER_OK) {
        return rserver;
    }

    return RSERVER_OK;
}

sres_code client_list(int s, char *filename) {
    method_code method = LIST;
    sres_code rserver;
    file_version v;
    int list_size;
    char req_filename[PATH_MAX];
    int counter = 0;
    int readed;

    // 1. Enviar el método
    if (write(s, &method, sizeof(method_code)) == -1) {
        return RSOCKET_ERROR;
    }

    memset(req_filename, 0, PATH_MAX);
    strcpy(req_filename, filename);

    // 2. Manda el nombre del archivo
    if (write(s, filename, PATH_MAX) == -1) {
        return RSOCKET_ERROR;
    }

    // 3. Recibe el tamaño de la lista a recibir
    if (read(s, &list_size, sizeof(int)) == -1) {
        return -1;
    }

    // 4. Recibe versiones hasta que se indique parar
    while (1) {
        if (read(s, &v, sizeof(file_version)) == -1) {
            return RSOCKET_ERROR;
        }

        if (filename != NULL) {
            printf("%i", ++counter);
            print_version(&v);
        } else {
            print_version(&v);
        }
    }

    return RSERVER_OK;
}

int server_receive_request(int s) {
    method_code method;
    int readed;

    // 1. recibe el método
    if ((readed = read(s, &method, sizeof(method_code))) == -1) {
        return -1;
    }
    if (readed != sizeof(method_code)) {
        return -1;
    }

    // 2. ejecuta el método
    switch (method) {
        case ADD:
            return server_add(s);
        case GET:
            return server_get(s);
        case LIST:
            return server_list(s);
        default:
            return -1;
    }
}

int server_add(int s) {
    struct add_request request;
    sres_code rserver;
    file_version v;

    // 1. recibe la peticion
    if (read(s, &request, sizeof(request)) == -1) {
        return -1;
    }

    // 2. comprueba si el archivo ya existe
    if (version_exists(request.filename, request.hash) ==
        VERSION_ALREADY_EXISTS) {
        rserver = RFILE_TO_DATE;
        return -1;
    } else {
        rserver = RSERVER_OK;
    }

    // 3. responde indicando si se debe de subir el archivo
    if (write(s, &rserver, sizeof(sres_code)) == -1) {
        return -1;
    }
    if (rserver != RSERVER_OK) {
        return -1;
    }

    // 4. recibe el archivo
    rserver = receive_file(s, request.filename);

    // 5. responde indicando si se pudo subir el archivo
    if (write(s, &rserver, sizeof(sres_code)) == -1) {
        return -1;
    }
    if (rserver != RSERVER_OK) {
        return -1;
    }

    // 6. agrega un nuevo registro al archivo versions.db
    if (create_version(request.filename, request.comment, &v) ==
        VERSION_ERROR) {
        return -1;
    };
    if (add_new_version(&v) == VERSION_ERROR) {
        return -1;
    };

    return 0;
}

int server_get(int s) {
    struct get_request request;
    sres_code rserver;
    cres_code rclient;
    file_version v;

    // 1. recibe la peticion
    if (read(s, &request, sizeof(request)) == -1) {
        return -1;
    }

    // 2. responde indicando si el archivo existe
    if (get_version(&v, request.filename, request.version) ==
        VERSION_NOT_FOUND) {
        rserver = RFILE_NOT_FOUND;
    } else {
        rserver = RSERVER_OK;
    }
    if (write(s, &rserver, sizeof(sres_code)) == -1) {
        return -1;
    }
    if (rserver != RSERVER_OK) {
        return -1;
    }

    // 4. responde con el hash de la versión
    if (write(s, v.hash, HASH_SIZE) == -1) {
        return -1;
    }

    // 5. recibe confirmación del cliente para descargar el archivo
    if (read(s, &rclient, sizeof(cres_code)) == -1) {
        return -1;
    }
    if (rclient != CONFIRM) {
        return -1;
    }

    // 6. envia el archivo
    rserver = send_file(s, request.filename);
    return 0;
}

int server_list(int s) {
    char *filename;

    // 1. recibe el nombre del archivo
    if (read(s, &filename, PATH_MAX) == -1) {
        return -1;
    }

    // 2. envia la lista
    if (send_versions(s, filename) == -1) {
        return -1;
    }

    return 0;
}

int send_file(int s, char *filename) {
    FILE *fp;
    char buf[BUFSZ];
    ssize_t nread;
    content_size file_size;

    // 0. Consulta el tamaño del archivo
    struct stat file_stat;
    if (stat(filename, &file_stat) == -1) {
        return -1;
    }
    if (file_stat.st_size > content_max) {
        return -1;
    }

    file_size = (content_size)file_stat.st_size;

    // 1. Envia el tamaño del archivo
    if (write(s, &file_size, sizeof(content_size)) == -1) {
        return -1;
    }

    // 2. Abre el archivo
    if ((fp = fopen(filename, "r")) == NULL) {
        perror("Error Abriendo el archivo");
        return -1;
    }

    // 3. Envia el contenido del archivo
    while (nread = fread(buf, sizeof(char), BUFSZ, fp), nread > 0) {
        if (write(s, buf, nread) == -1) {
            fclose(fp);
            return -1;
        }
    }
    fclose(fp);
    return 0;
}

int receive_file(int s, char *filename) {
    FILE *fp;
    content_size file_size;
    char buf[BUFSZ];
    ssize_t nread;

    // 0. Recibe el tamaño del archivo
    if (read(s, &file_size, sizeof(content_size)) == -1) {
        return -1;
    }

    // 1. Abre el archivo
    if ((fp = fopen(filename, "w")) == NULL) {
        perror("Error Abriendo el archivo");
        return -1;
    }

    // 2. Recibe el contenido del archivo
    while (nread = fread(buf, sizeof(char), BUFSZ, fp), nread > 0) {
        if (write(s, buf, nread) == -1) {
            fclose(fp);
            return -1;
        }
    }
    fclose(fp);
    return 0;
}

int send_versions(int s, char *filename) {
    FILE *fp;
    file_version v;
    int counter = 0;
    int readed;

    // Envia el tamaño de la lista
    if ((fp = fopen(VERSIONS_DIR "/" VERSIONS_DB, "r")) == NULL) {
        return -1;
    }
    while ((readed = fread(&v, sizeof(file_version), 1, fp)) == 1) {
        if (EQUALS(filename, v.filename)) counter++;
    }
    fclose(fp);
    if (write(s, &counter, sizeof(int)) == -1) {
        return -1;
    }
    if (counter == 0) {
        return 0;
    }

    // Envia la lista
    if ((fp = fopen(VERSIONS_DIR "/" VERSIONS_DB, "r")) == NULL) {
        return -1;
    }
    while ((readed = fread(&v, sizeof(file_version), 1, fp)) == 1) {
        if (EQUALS(filename, v.filename) &&
            write(s, &v, sizeof(file_version)) == -1) {
            fclose(fp);
            return -1;
        }
    }
    fclose(fp);

    return 0;
}
