/**
 * @file serverv.c
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andrés Martinez Varón <jorgeandre@unicauca.edu.co>
 * @brief Implementación de métodos del servidor
 * 
 * @copyright MIT License
 * 
 */
#include "serverv.h"

#include <arpa/inet.h>
#include <libgen.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "protocol.h"
#include "versions.h"

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

int server_receive_request(int s) {
    method_code method;
    int readed;
    sres_code rcode;

    // 1. recibe el método a ejecutar
    readed = read(s, &method, sizeof(method_code));
    if (readed != sizeof(method_code)) return RSOCKET_ERROR;

    // 2. ejecuta el método
    switch (method) {
        case ADD:
            printf("Cliente %d> ADD\n", s);
            rcode = server_add(s);
            break;
        case GET:
            printf("Cliente %d> GET\n", s);
            rcode = server_get(s);
            break;
        case LIST:
            printf("Cliente %d> LIST\n", s);
            rcode = server_list(s);
            break;
        case EXIT:
            printf("Cliente %d> EXIT\n", s);
            return 0;  // 0 es salir
        default:
            return RILLEGAL_METHOD;
    }
    if (rcode != RSERVER_OK) return -1;  // -1 es error
    return 1;                            // 1 es continuar
}

int server_add(int s) {
    struct add_request request;
    sres_code rserver;
    file_version v;
    ssize_t sent;  // bytes recibidos
    char dst_filename[PATH_MAX], buf[BUFSZ];

    // 1. recibe la peticion
    memset(&request, 0, sizeof(request));
    if (receive_string(s, request.filename, sizeof(request.filename)) == -1)
        return RSOCKET_ERROR;
    if (receive_string(s, request.hash, sizeof(request.hash)) == -1)
        return RSOCKET_ERROR;
    if (receive_string(s, request.comment, sizeof(request.comment)) == -1)
        return RSOCKET_ERROR;

    // 2. comprueba si el archivo ya existe
    puts("Comprobando version...");
    rserver =
        version_exists(request.filename, request.hash) == VERSION_ALREADY_EXISTS
            ? RFILE_TO_DATE
            : RSERVER_OK;

    // 3. responde indicando si se debe de subir el archivo
    sent = write(s, &rserver, sizeof(sres_code));
    if (sent != sizeof(sres_code)) return -1;
    if (rserver == RFILE_TO_DATE) return 0;

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
    return RSERVER_OK;
}

int server_get(int s) {
    struct get_request request;
    sres_code rserver;
    cres_code rclient;
    ssize_t received;
    ssize_t sent;
    file_version v;
    char filepath[PATH_MAX];
    char buf[BUFSZ];
    int aux;

    // 1. recibe la peticion (version y nombre del archivo)
    memset(&request, 0, sizeof(request));
    if (read(s, &request.version, sizeof(int)) != sizeof(int)) return -1;
    if (receive_string(s, request.filename, sizeof(request.filename)) == -1)
        return -1;

    // 2. responde indicando si el archivo existe
    rserver =
        get_version(&v, request.filename, request.version) == VERSION_NOT_FOUND
            ? RFILE_NOT_FOUND
            : RSERVER_OK;
    sent = write(s, &rserver, sizeof(sres_code));
    if (sent != sizeof(sres_code)) return -1;
    if (rserver == RFILE_NOT_FOUND) return 0;

    // 4. envía el hash de la versión
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
    return RSERVER_OK;
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

    return RSERVER_OK;
}

int send_versions(int s, char *filename) {
    FILE *fp;
    file_version v;
    cres_code rclient;
    char buf[BUFSZ];
    int counter;
    size_t readed;
    ssize_t sent;
    int aux;

    // Calcula el tamaño de la lista
    fp = fopen(VERSIONS_DIR "/" VERSIONS_DB, "r");
    if (fp == NULL) return -1;
    counter = 0;
    while (fread(&v, sizeof(file_version), 1, fp) == 1) {
        if (filename[0] == 0 || EQUALS(filename, v.filename)) counter++;
    }
    fclose(fp);

    // Envia el tamaño de la lista
    if (write(s, &counter, sizeof(int)) != sizeof(int)) return -1;

    if (counter == 0) return 0;

    // Envia la lista
    fp = fopen(VERSIONS_DIR "/" VERSIONS_DB, "r");
    if (fp == NULL) return -1;

    while (counter) {
        readed = fread(&v, sizeof(file_version), 1, fp);
        if (readed == 0 || readed == -1) break;

        if ((filename[0] == 0 || EQUALS(filename, v.filename))) {
            if (send_string(s, v.comment) == -1) break;
            if (send_string(s, v.filename) == -1) break;
            if (send_string(s, v.hash) == -1) break;
            counter--;
        }
    }
    fclose(fp);

    if (counter > 0) {
        return -1;
    }
    return 0;
}
