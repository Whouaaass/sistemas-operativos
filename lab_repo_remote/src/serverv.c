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
#include <linux/limits.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "protocol.h"
#include "userauth.h"
#include "versions.h"

/**
 * @brief Envia las versiones del repositorio al cliente
 *
 * @param s socket del cliente
 * @param filename nombre del archivo
 * @param session sesion del cliente
 * @return int 0 para exito, -1 para error
 */
int send_versions(int s, char *filename, user_session *session);

/**
 * @brief Obtiene la ruta completa al archivo de versiones del usuario
 *
 * @param session sesion del cliente
 * @param result ruta completa al archivo de versiones del usuario
 * @return char* ruta completa al archivo de versiones del usuario
 */
char *get_user_versionsdb_path(user_session *session, char *result);

/**
 * @brief Ejecuta el protocolo del servidor en el método add
 *
 * @param s socket del cliente
 * @param session session del cliente
 * @return sres_code respuesta mandada
 */
int server_add(int s, user_session *session);

/**
 * @brief Ejecuta el protocolo del servidor en el método get
 *
 * @param s socket del cliente
 * @param session session del cliente
 * @return sres_code respuesta mandada
 */
int server_get(int s, user_session *session);

/**
 * @brief Ejecuta el protocolo del servidor en el método list
 *
 * @param s socket del cliente
 * @param session session del cliente
 * @return sres_code respuesta mandada
 */
int server_list(int s, user_session *session);

/**
 * @brief autentifica un cliente
 *
 * @param s socket del cliente
 * @param username donde ser guarda el nombre del usuario
 * @return int 0 para caso de exito, -1 para error
 */
int authenticate_session(int s, char *username);

/**
 * @brief registra un usuario
 *
 * @param s socket del cliente
 * @param username donde se guarda el nombre del usuario
 * @return int 0 en caso de exito, -1 en caso de error
 */
int register_user(int s, char *username);

/**
 * @brief Manda una respuesta al cliente
 * @param s socket del cliente
 * @return int 0 en caso de exito, -1 en caso de error
 */
int send_server_response(int s, pres_code response);

int server_receive_request(int s, user_session *session) {
    method_code method;
    int readed;
    pres_code rcode;

    // 1. recibe el método a ejecutar
    readed = read(s, &method, sizeof(method_code));
    if (readed != sizeof(method_code)) return RSOCKET_ERROR;

    // 2. ejecuta el método
    switch (method) {
        case ADD:
            printf("Cliente %d> ADD\n", s);
            if (session->authenticated == 0)
                return send_server_response(s, RDENIED) == 0 ? 1 : -1;
            if (send_server_response(s, RSERVER_OK) == -1) return -1;
            rcode = server_add(s, session);
            break;
        case GET:
            printf("Cliente %d> GET\n", s);
            if (session->authenticated == 0)
                return send_server_response(s, RDENIED) == 0 ? 1 : -1;
            if (send_server_response(s, RSERVER_OK) == -1) return -1;
            rcode = server_get(s, session);
            break;
        case LIST:
            printf("Cliente %d> LIST\n", s);
            if (session->authenticated == 0)
                return send_server_response(s, RDENIED) == 0 ? 1 : -1;
            if (send_server_response(s, RSERVER_OK) == -1) return -1;
            rcode = server_list(s, session);
            break;
        case EXIT:
            printf("Cliente %d> EXIT\n", s);
            return 0;  // 0 es salir
        case LOGIN:
            printf("Cliente %d> LOGIN\n", s);
            if (send_server_response(s, RSERVER_OK) == -1) return -1;
            rcode = authenticate_session(s, session->username);
            session->authenticated = rcode == RSERVER_OK;
            if (rcode != RSOCKET_ERROR) return 1;
            break;
        case REGISTER:
            printf("Cliente %d> REGISTER\n", s);
            if (send_server_response(s, RSERVER_OK) == -1) return -1;
            rcode = register_user(s, session->username);
            session->authenticated = rcode == RSERVER_OK;
            if (rcode != RSOCKET_ERROR) return 1;
            break;
        default:
            if (send_server_response(s, RSERVER_OK) == -1) return -1;
            return 1;
    }
    if (rcode != RSERVER_OK) return -1;  // -1 es error
    return 1;                            // 1 es continuar
}

int server_add(int s, user_session *session) {
    struct add_request request;
    pres_code rserver;
    file_version v;
    ssize_t sent;  // bytes recibidos
    char filename_buf[PATH_MAX], buf[BUFSZ];

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
        version_exists(request.filename, request.hash, get_user_versionsdb_path(session, filename_buf)) == VERSION_ALREADY_EXISTS
            ? RFILE_TO_DATE
            : RSERVER_OK;

    // 3. responde indicando si se debe de subir el archivo
    sent = write(s, &rserver, sizeof(pres_code));
    if (sent != sizeof(pres_code)) return -1;
    if (rserver == RFILE_TO_DATE) return 0;

    // 4. recibe el archivo
    puts("Recibiendo archivo...");
    snprintf(filename_buf, PATH_MAX, "%s/%s", VERSIONS_DIR, request.hash);
    rserver = receive_file(s, filename_buf) == 0 ? RSERVER_OK : RERROR;

    // 5. responde indicando si se pudo subir el archivo
    sent = write(s, &rserver, sizeof(pres_code));
    if (sent != sizeof(pres_code)) return -1;
    if (rserver == -1) return -1;

    // 6. agrega un nuevo registro al archivo versions.db
    memset(&v, 0, sizeof(file_version));
    strcpy(v.filename, request.filename);
    strcpy(v.comment, request.comment);
    strcpy(v.hash, request.hash);

    if (add_new_version(&v, get_user_versionsdb_path(session, filename_buf)) ==
        VERSION_ERROR) {
        return -1;
    };

    puts("Archivo agregado!");
    return RSERVER_OK;
}

int server_get(int s, user_session *session) {
    struct get_request request;
    pres_code rserver;
    cres_code rclient;
    ssize_t received;
    ssize_t sent;
    file_version v;
    char filepath_buf[PATH_MAX];
    char buf[BUFSZ];
    int aux;

    // 1. recibe la peticion (version y nombre del archivo)
    memset(&request, 0, sizeof(request));
    if (read(s, &request.version, sizeof(int)) != sizeof(int)) return -1;
    if (receive_string(s, request.filename, sizeof(request.filename)) == -1)
        return -1;

    // 2. responde indicando si el archivo existe
    rserver = get_version(&v, request.filename, request.version,
                          get_user_versionsdb_path(session, filepath_buf)) ==
                      VERSION_NOT_FOUND
                  ? RFILE_NOT_FOUND
                  : RSERVER_OK;
    sent = write(s, &rserver, sizeof(pres_code));
    if (sent != sizeof(pres_code)) return -1;
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
    snprintf(filepath_buf, PATH_MAX, VERSIONS_DIR "/%s", v.hash);
    if (send_file(s, filepath_buf) == -1) return -1;

    printf("Archivo %s enviado!\n", request.filename);
    return RSERVER_OK;
}

int server_list(int s, user_session *session) {
    char filename[PATH_MAX];
    char buf[BUFSZ];
    int aux;

    // 1. recibe el nombre del archivo
    if (read(s, &buf, BUFSZ) != BUFSZ) {
        return -1;
    }
    strcpy(filename, buf);

    // 2. envia la lista
    if (send_versions(s, filename, session) == -1) {
        return -1;
    }
    puts("Lista enviada!");

    return RSERVER_OK;
}

int send_versions(int s, char *filename, user_session *session) {
    FILE *fp;
    file_version v;
    int counter;
    char versions_path[PATH_MAX];
    get_user_versionsdb_path(session, versions_path);
    // Calcula el tamaño de la lista
    fp = fopen(versions_path, "r");

    counter = 0;
    if (fp != NULL) {
        while (fread(&v, sizeof(file_version), 1, fp) == 1) {
            if (filename[0] == 0 || EQUALS(filename, v.filename)) counter++;
        }
        fclose(fp);
    }

    // Envia el tamaño de la lista
    if (write(s, &counter, sizeof(int)) != sizeof(int)) return -1;

    if (counter == 0) return 0;

    // Envia la lista
    fp = fopen(versions_path, "r");
    if (fp == NULL) return -1;

    while (counter) {
        size_t readed = fread(&v, sizeof(file_version), 1, fp);
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

int authenticate_session(int s, char *username) {
    struct user_auth_request req;
    ssize_t received;
    user_record user;
    pres_code rcode;
    cres_code cres;
    ssize_t sent;

    memset(&req, 0, sizeof(req));
    // 1. recibe la petición
    received = receive_data(s, &req, sizeof(struct user_auth_request));

    // 3. autentifica al usuario
    if (search_user(req.username, &user) == 0) {
        rcode = EQUALS(user.password, req.password) ? RSERVER_OK : RDENIED;
    } else {
        rcode = RUSER_NOT_FOUND;
    }

    // 5. responde con el codigo de respuesta
    sent = write(s, &rcode, sizeof(pres_code));
    if (sent != sizeof(pres_code)) return RSOCKET_ERROR;

    if (rcode == RSERVER_OK) {
        printf("Usuario %s autentificado\n", req.username);
        memset(username, 0, USERNAME_SIZE);
        strcpy(username, req.username);
    }
    return rcode;
}

int register_user(int s, char *username) {
    struct user_auth_request req;
    ssize_t received;
    user_record user;
    pres_code rcode;
    cres_code cres;
    ssize_t sent;

    // 1. recibe la petición (datos de usuario)
    received = receive_data(s, &req, sizeof(struct user_auth_request));

    // 3. busca si el usuario existe o no
    if (search_user(req.username, &user) == -1) {
        // crea el usuario si no existe
        save_user(req.username, req.password, s);
        rcode = RSERVER_OK;
    } else {
        rcode = RUSER_ALREADY_EXISTS;
    }

    // 5. responde con el codigo de respuesta
    sent = write(s, &rcode, sizeof(pres_code));
    if (sent != sizeof(pres_code)) return RSOCKET_ERROR;
    if (rcode == RSERVER_OK) {
        printf("Usuario %s registrado\n", req.username);
        memset(username, 0, USERNAME_SIZE);
        strcpy(username, req.username);
        return 0;
    } else {
        return rcode;
    }
}

int send_server_response(int s, pres_code response) {
    ssize_t sent;
    sent = write(s, &response, sizeof(pres_code));
    if (sent != sizeof(pres_code)) return -1;
    return 0;
}

char *get_user_versionsdb_path(user_session *session, char *result) {
    snprintf(result, PATH_MAX, VERSIONS_DIR "/versions-%s.db",
             session->username);
    return result;
}