/**
 * @file
 * @brief Sistema de control de versiones CLIENTE
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andrés Martinez Varón <jorgeandre@unicauca.edu.co>
 * @copyright MIT License
 */
#include <arpa/inet.h>
#include <libgen.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "protocol.h"
#include "versions.h"

// TODO: search a method to save this on a file or something like that
#define ip_addr "192.168.100.147"
#define port 1234

/**
 * @brief Crea la conección con el servidor
 *
 * @return int the socket con el servidor, -1 para errores
 */
int make_connection();

/**
 * @brief Imprime el mensaje de ayuda
 */
void usage();

/**
 * @brief El manejador de señales
 */
void handle_signal(int sig);

int main(int argc, char *argv[]) {
    int s;           // socket con el servidor
    int rcode;       // codigo de retorno
    char *filename;  // nombre del archivo

    // 1. instalar los manejadores de SIGINT, SIGTERM
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // 2. Maneja los argumentos
    if (argc == 4 && EQUALS(argv[1], "add")) {
        s = make_connection();
        filename = basename(argv[2]);
        rcode = client_list(s, filename);
        if (rcode != RSERVER_OK) {
            perror("Error listing files");
            exit(EXIT_FAILURE);
        }
    } else if (argc == 2 && EQUALS(argv[1], "list")) {
        // Listar todos los archivos almacenados en el repositorio
        s = make_connection();
        rcode = client_list(s, NULL);
        if (rcode != RSERVER_OK) {
            perror("Error listing files");
            exit(EXIT_FAILURE);
        }

    } else if (argc == 3 && EQUALS(argv[1], "list")) {
        // Listar el archivo solicitado
        s = make_connection();
        filename = basename(argv[2]);
        rcode = client_list(s, filename);
        if (rcode != RSERVER_OK) {
            perror("Error listing files");
            exit(EXIT_FAILURE);
        }
    } else if (argc == 4 && EQUALS(argv[1], "get")) {
        int v_number = atoi(argv[2]);
        filename = basename(argv[3]);
        if ((s = make_connection()) == -1) {
            perror("Error connecting to server");
            exit(EXIT_FAILURE);
        }
        rcode = client_get(s, filename, v_number);
        if (rcode != RSERVER_OK) {
            perror("Error getting file");
            exit(EXIT_FAILURE);
        }
    } else {
        usage();
    }

    // 3. cerrar la conexión con el servidor - close
    close(s);
    exit(EXIT_SUCCESS);
}

int make_connection() {
    int s;  // socket del servidor
    struct sockaddr_in addr;

    // 1. obtener un conector - socket
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return -1;
    }

    // 2. conectarse a una dirección (IP, puerto) - connect ~ bind
    memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    if (inet_aton(ip_addr, &addr.sin_addr) == 0) {
        return -1;
    }
    addr.sin_port = htons(port);

    puts("Connecting to server...");
    if (connect(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_in))) {
        return -1;
    }

    // 3. Hacer el protocolo del saludo
    if (send_greeting(s, 1) == -1) {
        return -1;
    }
    if (receive_greeting(s, 1) == -1) {
        return -1;
    }

    return s;
}

void usage() { puts("TODO: make usage"); }

void handle_signal(int sig) {
    if (sig == SIGTERM) {
        puts("Closed by terminal signal");
    }
    if (sig == SIGINT) {
        puts("Closed by keyboard");
    }
    puts("Closing...");
    exit(EXIT_FAILURE);
}
