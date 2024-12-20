/**
 * @file clientv.c
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andrés Martinez Varón <jorgeandre@unicauca.edu.co>
 * @brief Implementación de los métodos del cliente
 *
 * @copyright MIT License
 *
 */

#include "clientv.h"

#include <arpa/inet.h>
#include <libgen.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "protocol.h"
#include "userauth.h"
#include "versions.h"

pres_code client_add(int s, char *filename, char *comment) {
    const method_code method = ADD;
    struct add_request request;
    struct stat file_stat;
    ssize_t received;  // bytes recibidos y enviados
    char hash[HASH_SIZE];
    pres_code rserver;

    // 0. Comprobar que el archivo exista
    if (stat(filename, &file_stat) != 0) return RFILE_NOT_FOUND;

    // 1. Enviar el método
    if (write(s, &method, sizeof(method_code)) == -1) return RSOCKET_ERROR;

    // recibe confirmación de que el método es correcto
    received = read(s, &rserver, sizeof(pres_code));
    if (received != sizeof(pres_code)) return RSOCKET_ERROR;
    if (rserver != RSERVER_OK) return rserver;

    // 2. Enviar el nombre del archivo, hash y comentario
    puts("Obteniendo hash...");
    get_file_hash(filename, hash);

    if (send_string(s, filename) == -1) return RSOCKET_ERROR;
    if (send_string(s, hash) == -1) return RSOCKET_ERROR;
    if (send_string(s, comment) == -1) return RSOCKET_ERROR;

    // 3. Recibir una respuesta del servidor (indica si se puede subir el
    // archivo)
    received = read(s, &rserver, sizeof(pres_code));
    if (received != sizeof(pres_code)) return RSOCKET_ERROR;
    if (rserver != RSERVER_OK) return rserver;

    // 4. Manda el archivo
    puts("Enviando archivo...");
    if (send_file(s, filename) == -1) {
        return RERROR;
    }

    // 5. Recibe la respuesta del servidor
    received = read(s, &rserver, sizeof(pres_code));
    if (received != sizeof(pres_code)) return RSOCKET_ERROR;
    if (rserver != RSERVER_OK) return rserver;

    return RSERVER_OK;
}

pres_code client_get(int s, char *filename, int version) {
    const method_code method = GET;
    struct get_request request;
    pres_code rserver;
    cres_code cres;
    ssize_t sent, received;
    char server_hash[HASH_SIZE];
    char local_hash[HASH_SIZE];
    char buf[BUFSZ];

    // 1. Enviar el método
    sent = write(s, &method, sizeof(method_code));
    if (sent != sizeof(method_code)) return RSOCKET_ERROR;

    // recibe confirmación de que el método es correcto
    received = read(s, &rserver, sizeof(pres_code));
    if (received != sizeof(pres_code)) return RSOCKET_ERROR;
    if (rserver != RSERVER_OK) return rserver;

    // 2. Enviar la versión y el nombre del archivo
    sent = write(s, &version, sizeof(int));
    if (sent != sizeof(int)) return RSOCKET_ERROR;
    if (send_string(s, filename) == -1) return RSOCKET_ERROR;

    // 3. Recibe la respuesta del servidor indicando si el archivo existe
    received = read(s, &rserver, sizeof(pres_code));
    if (received != sizeof(pres_code)) return RSOCKET_ERROR;
    if (rserver != RSERVER_OK) return rserver;

    // 4. Recibe el hash de la versión
    received = read(s, server_hash, HASH_SIZE);
    if (received != HASH_SIZE) return RSOCKET_ERROR;

    // 5. responde si el archivo local ya esta actualizado
    cres = get_file_hash(filename, local_hash) != NULL &&
                   EQUALS(server_hash, local_hash)
               ? DENY
               : CONFIRM;
    sent = write(s, &cres, sizeof(cres_code));
    if (sent != sizeof(cres_code)) return RSOCKET_ERROR;
    if (cres == DENY) {
        puts("El archivo ya existe");
        return RFILE_TO_DATE;
    }

    puts("Recibiendo archivo...");
    // 6. Recibe el archivo
    rserver = receive_file(s, filename);
    if (rserver != RSERVER_OK) {
        return rserver;
    }
    printf("Archivo %s descargado correctamente\n", filename);
    return RSERVER_OK;
}

pres_code client_list(int s, char *filename) {
    method_code method = LIST;
    pres_code rserver;
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

    // recibe confirmación de que el método es correcto
    received = read(s, &rserver, sizeof(pres_code));
    if (received != sizeof(pres_code)) return RSOCKET_ERROR;
    if (rserver != RSERVER_OK) return rserver;

    memset(buf, 0, BUFSZ);
    if (filename != 0) snprintf(buf, BUFSZ, "%s", filename);

    // 2. Manda el nombre del archivo
    if (write(s, buf, BUFSZ) != BUFSZ) {
        return RSOCKET_ERROR;
    }

    // 3. Recibe el tamaño de la lista
    received = recv(s, &list_size, sizeof(int), 0);
    if (received != sizeof(int)) return RSOCKET_ERROR;

    // 4. Recibe la lista de versiones // 4
    while (list_size--) {
        memset(&v, 0, sizeof(file_version));
        if (receive_string(s, v.comment, sizeof(v.comment)) == -1)
            return RSOCKET_ERROR;
        if (receive_string(s, v.filename, sizeof(v.filename)) == -1)
            return RSOCKET_ERROR;
        if (receive_string(s, v.hash, sizeof(v.hash)) == -1)
            return RSOCKET_ERROR;

        if (filename != NULL) {
            printf("%i ", ++counter);
            print_version(&v);
        } else {
            print_version(&v);
        }
    }

    return RSERVER_OK;
}

int authenticate_session(int s, char *username, char *password) {
    method_code method = LOGIN;
    struct user_auth_request request;
    ssize_t s_size;
    pres_code rserver;
    cres_code cres;

    // manda el método que se desea ejecutar
    s_size = write(s, &method, sizeof(method_code));
    if (s_size != sizeof(method_code)) return -1;

    // recibe confirmación de que el método es correcto
    if (read(s, &rserver, sizeof(pres_code)) != sizeof(pres_code))
        return RSOCKET_ERROR;
    if (rserver != RSERVER_OK) return rserver;

    memset(&request, 0, sizeof(struct user_auth_request));
    snprintf(request.username, USERNAME_SIZE, "%s", username);
    snprintf(request.password, PASSWORD_SIZE, "%s", password);

    // 1. envia la petición
    if (send_data(s, &request, sizeof(struct user_auth_request)) == -1)
        return -1;

    // 2. recibe el codigo de respuesta
    s_size = read(s, &rserver, sizeof(pres_code));
    if (s_size != sizeof(pres_code)) return -1;

    return rserver;
}

int register_user(int s, char *username, char *password) {
    const method_code method = REGISTER;
    struct user_auth_request request;
    ssize_t s_size;
    pres_code rserver;
    cres_code cres;

    // manda el método que se desea ejecutar
    s_size = write(s, &method, sizeof(method_code));
    if (s_size != sizeof(method_code)) return -1;

    // recibe confirmación de que el método es correcto
    if (read(s, &rserver, sizeof(pres_code)) != sizeof(pres_code))
        return RSOCKET_ERROR;
    if (rserver != RSERVER_OK) return rserver;

    memset(&request, 0, sizeof(request));
    snprintf(request.username, USERNAME_SIZE, "%s", username);
    snprintf(request.password, PASSWORD_SIZE, "%s", password);

    // 1. envia la petición
    if (send_data(s, &request, sizeof(struct user_auth_request)) == -1)
        return -1;

    // 2. recibe el codigo de respuesta
    s_size = read(s, &rserver, sizeof(pres_code));
    if (s_size != sizeof(pres_code)) return -1;

    return rserver;
}