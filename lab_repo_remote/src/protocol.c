/**
 * @file protocol.c
 * @brief Implementación del protocolo de comunicación
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andrés Martinez Varón <jorgeandre@unicauca.edu.co>
 * @copyright MIT License
 */
#include "protocol.h"

#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


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
    if (stat(filename, &file_stat) == -1) return -1;
    if (file_stat.st_size > content_max) return -1;

    file_size = (content_size)file_stat.st_size;

    // 1. Envia el tamaño del archivo
    sent = write(s, &file_size, sizeof(content_size));
    if (sent != sizeof(content_size)) return -1;

    // 2. Abre el archivo
    if ((fp = fopen(filename, "r")) == NULL) {
        perror("Error Abriendo el archivo");
        return -1;
    }

    flockfile(fp);

    // 3. Envia el contenido del archivo
    sent = 0;
    while (file_size -= sent) {
        nread = fread(buf, sizeof(char), BUFSZ, fp);
        if (nread == 0 || nread == -1) break;
        sent = send(s, buf, nread, 0);
        if (sent == -1 || sent != nread) break;
    }

    funlockfile(fp);
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
    received = read(s, &file_size, sizeof(content_size));
    if (received != sizeof(content_size)) return -1;

    // 1. Abre el archivo
    if ((fp = fopen(endpath, "wb")) == NULL) {
        perror("Error Abriendo el archivo");
        return -1;
    }

    flockfile(fp);
    // 2. Recibe el contenido del archivo
    received = 0;
    while (file_size -= received) {
        received = recv(s, buf, BUFSZ, 0);
        if (received == 0 || received == -1) break;
        nwrite = fwrite(buf, sizeof(char), received, fp);
        if (nwrite != received) break;
    }

    funlockfile(fp);
    fclose(fp);
    if (file_size != 0) return -1;
    return 0;
}

int send_data(int s, void *data, size_t size) {
    char *out_ptr = malloc(size);
    if (out_ptr == NULL) return -1;
    char *ptr = out_ptr;
    memcpy(out_ptr, data, size);
    ssize_t to_send = size;

    while (to_send > 0) {
        ssize_t nsent = send(s, ptr, to_send, 0);
        if (nsent == -1) return -1;
        to_send -= nsent;
        ptr += nsent;
    }
    free(out_ptr);
    return 0;
}

int receive_data(int s, void *data, size_t size) {
    char *in_ptr = malloc(size);
    if (in_ptr == NULL) return -1;
    char *ptr = in_ptr;
    ssize_t to_receive = size;
    while (to_receive > 0) {
        ssize_t nreceived = recv(s, ptr, to_receive, 0);
        if (nreceived == -1) return -1;
        to_receive -= nreceived;
        ptr += nreceived;
    }
    memcpy(data, in_ptr, size);
    free(in_ptr);
    return 0;
}

int send_string(int s, char *str) {
    size_t size = strlen(str);
    if (send(s, &size, sizeof(size_t), 0) != sizeof(size_t)) return -1;
    while (size) {
        ssize_t nsent = send(s, str, size, 0);
        if (nsent == -1) return -1;
        size -= nsent;
        str += nsent;
    }
    return 0;
}

int receive_string(int s, char *str, size_t max_size) {
    size_t to_receive;
    if (recv(s, &to_receive, sizeof(size_t), 0) != sizeof(size_t)) return -1;

    if (to_receive > max_size) {
        errno = E2BIG; /* Argument list too long */
        return -1;
    }

    while (to_receive) {
        ssize_t nreceived = recv(s, str, to_receive, 0);
        if (nreceived == -1) return -1;
        to_receive -= nreceived;
        str += nreceived;
    }

    return 0;
}

char *get_protocol_rmsg(pres_code code) {
    switch (code) {
        case RFILE_TO_DATE:
            return "El archivo ya está actualizado";
        case RFILE_OUTDATED:
            return "El archivo no esta actualizado";
        case RFILE_NOT_FOUND:
            return "El archivo no existe";
        case RVERSION_NOT_FOUND:
            return "La versión solicitada no existe";
        case RSOCKET_ERROR:
            return "Error de socket (En escritura o lectura)";
        case RILLEGAL_METHOD:
            return "Método no permitido";
        case RERROR:
            perror("Error no especificado");
            return "Error no especificado";
        case RDENIED:
            return "Acceso denegado";
        case RUSER_NOT_FOUND:
            return "Usuario no encontrado";
        case RUSER_ALREADY_EXISTS:
            return "Usuario ya existe";
        default:
            perror("Error desconocido");
            return "Error desconocido";
    }
}