/**
 * @file
 * @brief Implementación del protocolo de comunicación
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andrés Martinez Varón <jorgeandre@unicauca.edu.co>
 * @copyright MIT License
 */
#include "protocol.h"

#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "versions.h"

/**
 * @brief Manda un archivo al socket de destino
 *
 * @param s socket de destino
 * @param filepath ruta donde se encuentra el archivo
 * @return int 0 en caso de exito, -1 en caso de error
 */
int send_file(int s, char *filepath);

/**
 * @brief Recibe un archivo del socket
 * @param s socket del que se recibe el archivo
 * @param filepath ruta en donde se guardará el archivo
 * @return int 0 en caso de exito, -1 en caso de error
 */
int receive_file(int s, char *filepath);

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
    ssize_t received, sent;  // bytes recibidos y enviados
    char hash[HASH_SIZE], buf[BUFSZ];
    sres_code rserver;
    struct stat file_stat;

    // 1. Enviar el método
    if (write(s, &method, sizeof(method_code)) == -1) {
        return RSOCKET_ERROR;
    }

    // 2. Enviar el nombre del archivo, hash y comentario
    puts("Obteniendo hash...");
    get_file_hash(filename, hash);

    memset(buf, 0, BUFSZ);
    snprintf(buf, BUFSZ, "%s", filename);
    sent = write(s, buf, BUFSZ);
    if (sent != BUFSZ) return RSOCKET_ERROR;
    memset(buf, 0, BUFSZ);
    snprintf(buf, BUFSZ, "%s", hash);
    sent = write(s, buf, BUFSZ);
    if (sent != BUFSZ) return RSOCKET_ERROR;
    memset(buf, 0, BUFSZ);
    snprintf(buf, BUFSZ, "%s", comment);
    sent = write(s, buf, BUFSZ);
    if (sent != BUFSZ) return RSOCKET_ERROR;

    /*
    memset(&request, 0, sizeof(request));
    strcpy(request.filename, filename);
    strcpy(request.comment, comment);
    strcpy(request.hash, hash);

    if (write(s, &request, sizeof(request)) == -1) {
        return RSOCKET_ERROR;
    }
    */

    // 3. Recibir una respuesta del servidor (indica si se puede subir el
    // archivo)
    if (read(s, &rserver, sizeof(sres_code)) == -1) {
        return RSOCKET_ERROR;
    }
    if (rserver != RSERVER_OK) {
        return rserver;
    }

    // 4. Manda el archivo
    puts("Enviando archivo...");
    if (send_file(s, filename) == -1) {
        return RERROR;
    }

    // 5. Recibe la respuesta del servidor
    received = read(s, &rserver, sizeof(sres_code));
    if (received != sizeof(sres_code)) return RSOCKET_ERROR;
    if (rserver != RSERVER_OK) return rserver;   
    

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
    if (cres == DENY) {
        puts("file up to date");
        return RFILE_TO_DATE;
    }

    // 6. Recibe el archivo
    rserver = receive_file(s, filename);
    if (rserver != RSERVER_OK) {
        return rserver;
    }
    puts("file getted sucessfully");
    return RSERVER_OK;
}

sres_code client_list(int s, char *filename) {
    method_code method = LIST;
    sres_code rserver;
    cres_code rclient;
    file_version v;
    char buf[BUFSZ];
    ssize_t received;
    ssize_t to_receive;
    int list_size;
    char req_filename[PATH_MAX];
    int counter = 0;
    int readed;

    // 1. Enviar el método
    if (write(s, &method, sizeof(method_code)) == -1) {
        return RSOCKET_ERROR;
    }

    memset(buf, 0, BUFSZ);
    if (filename != 0) snprintf(buf, BUFSZ, "%s", filename);

    // 2. Manda el nombre del archivo
    if (write(s, buf, BUFSZ) != BUFSZ) {
        return RSOCKET_ERROR;
    }

    // 3. Recibe el tamaño de la lista a recibir
    received = recv(s, &list_size, sizeof(int), 0);
    if (received != sizeof(int)) return RSOCKET_ERROR;

    // 4. Recibe versiones hasta que se indique parar
    while (list_size--) {
        received = recv(s, v.comment, sizeof(v.comment), 0);
        if (received != sizeof(v.comment)) return RSOCKET_ERROR;
        memset(buf, 0, BUFSZ);
        received = recv(s, buf, BUFSZ, 0);
        if (received != BUFSZ) return RSOCKET_ERROR;
        strcpy(v.filename, buf);
        received = recv(s, &v.hash, sizeof(v.hash), 0);
        if (received != sizeof(v.hash)) return RSOCKET_ERROR;

        if (filename != NULL) {
            printf("%i ", ++counter);
            print_version(&v);
        } else {
            print_version(&v);
        }
    }

    // Envia confirmación de que ha terminado de ver la lista
    rclient = CONFIRM;
    if (send(s, &rclient, sizeof(cres_code), 0) != sizeof(cres_code)) {
        return -1;
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
    ssize_t received, sent;  // bytes recibidos y enviados
    char dst_filename[PATH_MAX], buf[BUFSZ];

    // 1. recibe la peticion (Se tubo que partir por problemas de envio de
    // bytes)
    memset(&request, 0, sizeof(request));
    memset(buf, 0, BUFSZ);
    received = read(s, &buf, BUFSZ);
    if (received != BUFSZ) return -1;
    strcpy(request.filename, buf);
    memset(buf, 0, BUFSZ);
    received = read(s, &buf, BUFSZ);
    if (received != BUFSZ) return -1;
    strcpy(request.hash, buf);
    memset(buf, 0, BUFSZ);
    received = read(s, &buf, BUFSZ);
    if (received != BUFSZ) return -1;
    strcpy(request.comment, buf);

    // 2. comprueba si el archivo ya existe
    puts("Comprobando version...");
    if (version_exists(request.filename, request.hash) ==
        VERSION_ALREADY_EXISTS) {
        rserver = RFILE_TO_DATE;
        return -1;
    } else {
        rserver = RSERVER_OK;
    }

    // 3. responde indicando si se debe de subir el archivo
    sent = write(s, &rserver, sizeof(sres_code));
    if (sent != sizeof(sres_code)) return -1;
    if (rserver != RSERVER_OK) return -1;

    // 4. recibe el archivo
    puts("Recibiendo archivo...");
    snprintf(dst_filename, PATH_MAX, "%s/%s", VERSIONS_DIR, request.hash);
    rserver = receive_file(s, dst_filename) == 0 ? RSERVER_OK : RERROR;

    // 5. responde indicando si se pudo subir el archivo
    sent = write(s, &rserver, sizeof(sres_code));
    if (sent != sizeof(sres_code)) return -1;
    if (rserver == -1) return -1;

    // 6. agrega un nuevo registro al archivo versions.db
    memset(&v, 0, sizeof(file_version));
    strcpy(v.filename, request.filename);
    strcpy(v.comment, request.comment);
    strcpy(v.hash, request.hash);

    if (add_new_version(&v) == VERSION_ERROR) {
        return -1;
    };

    puts("Archivo agregado!");
    return 0;
}

int server_get(int s) {
    struct get_request request;
    sres_code rserver;
    cres_code rclient;

    ssize_t received;
    ssize_t sent;
    file_version v;
    char filepath[PATH_MAX];
    int aux;

    // 1. recibe la peticion
    received = read(s, &request, sizeof(request));
    if (received != sizeof(request)) return -1;

    // 2. responde indicando si el archivo existe
    if (get_version(&v, request.filename, request.version) ==
        VERSION_NOT_FOUND) {
        rserver = RFILE_NOT_FOUND;
    } else {
        rserver = RSERVER_OK;
    }
    sent = write(s, &request, sizeof(request));
    if (sent != sizeof(request)) return -1;
    if (rserver != RSERVER_OK) return -1;

    // 4. responde con el hash de la versión
    sent = write(s, v.hash, HASH_SIZE);
    if (sent != HASH_SIZE) return -1;

    // 5. recibe confirmación del cliente para descargar el archivo
    received = read(s, &rclient, sizeof(cres_code));
    if (received != sizeof(cres_code)) return -1;
    if (rclient != CONFIRM) return 0;

    // 6. envia el archivo
    puts("Enviando archivo...");
    snprintf(filepath, PATH_MAX, "%s/%s", VERSIONS_DIR, v.hash);
    if (send_file(s, filepath) == -1) return -1;

    printf("Archivo %s enviado!\n", request.filename);
    return 0;
}

int server_list(int s) {
    char filename[PATH_MAX];
    char buf[BUFSZ];
    int aux;

    // 1. recibe el nombre del archivo
    if (read(s, &buf, BUFSZ) != BUFSZ) {
        return -1;
    }
    strcpy(filename, buf);

    // 2. envia la lista
    if (send_versions(s, filename) == -1) {
        return -1;
    }
    puts("Lista enviada!");

    return 0;
}

int send_file(int s, char *filename) {
    FILE *fp;
    char buf[BUFSZ];
    ssize_t nread = 0;
    content_size file_size;
    ssize_t sent;
    int aux;

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
    if ((aux = write(s, &file_size, sizeof(content_size))) == -1 || aux == 0) {
        return -1;
    }

    // 2. Abre el archivo
    if ((fp = fopen(filename, "r")) == NULL) {
        perror("Error Abriendo el archivo");
        return -1;
    }

    // 3. Envia el contenido del archivo
    while (file_size -= nread) {
        nread = fread(buf, sizeof(char), BUFSZ, fp);
        if (nread == 0 || nread == -1) break;
        file_size -= nread;
        sent = send(s, buf, nread, 0);
        if (sent == -1 || sent != nread) break;
    }

    fclose(fp);
    if (file_size != 0) return -1;
    return 0;
}

int receive_file(int s, char *endpath) {
    FILE *fp;
    content_size file_size;
    char buf[BUFSZ];
    size_t nwrite;
    ssize_t received;

    // 0. Recibe el tamaño del archivo
    if (read(s, &file_size, sizeof(content_size)) != sizeof(content_size)) {
        return -1;
    }

    // 1. Abre el archivo
    if ((fp = fopen(endpath, "wb")) == NULL) {
        perror("Error Abriendo el archivo");
        return -1;
    }

    // 2. Recibe el contenido del archivo
    while (file_size > 0) {
        int rchunk_size = file_size < 0 ? BUFSZ : file_size;
        memset(buf, 0, BUFSZ);
        received = recv(s, buf, rchunk_size, 0);
        if (received == -1) break;
        file_size -= received;
        nwrite = fwrite(buf, sizeof(char), received, fp);
        if (nwrite != received) break;
    }

    fclose(fp);
    if (file_size != 0) return -1;
    return 0;
}

int send_versions(int s, char *filename) {
    FILE *fp;
    file_version v;
    cres_code rclient;
    char buf[BUFSZ];
    int counter = 0;
    size_t readed;
    ssize_t sent;
    int aux;

    // Envia el tamaño de la lista
    fp = fopen(VERSIONS_DIR "/" VERSIONS_DB, "r");
    if (fp == NULL) return -1;

    while ((readed = fread(&v, sizeof(file_version), 1, fp)) == 1) {
        if (filename[0] == 0)
            counter++;
        else if (EQUALS(filename, v.filename))
            counter++;
    }
    fclose(fp);
    if (write(s, &counter, sizeof(int)) == -1) return -1;

    if (counter == 0) return 0;

    // Envia la lista
    fp = fopen(VERSIONS_DIR "/" VERSIONS_DB, "r");
    if (fp == NULL) return -1;

    while (1) {
        readed = fread(&v, sizeof(file_version), 1, fp);
        if (readed == 0) break;
        if (readed == -1) return -1;
        if ((filename[0] == 0 || EQUALS(filename, v.filename))) {
            sent = send(s, v.comment, sizeof(v.comment), 0);
            if (sent != sizeof(v.comment)) return -1;
            memset(buf, 0, BUFSZ);
            strcpy(buf, v.filename);
            sent = send(s, buf, BUFSZ, 0);
            if (sent != BUFSZ) return -1;
            sent = send(s, &v.hash, sizeof(v.hash), 0);
            if (sent != sizeof(v.hash)) return -1;
        }
    }

    // Espera a que el cliente termine de ver la lista
    if (recv(s, &rclient, sizeof(cres_code), 0) != sizeof(cres_code)) {
        return -1;
    }

    fclose(fp);
    return 0;
}

void version_to_stream() {}