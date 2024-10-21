/**
 * @file
 * @brief Sistema de control de versiones SERVER
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andrés Martinez Varón <jorgeandre@unicauca.edu.co>
 * @copyright MIT License
 */

#include <arpa/inet.h>
#include <bits/pthreadtypes.h>
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

#define max_clients 2

/**
 * @brief Imprime el mensaje de ayuda
 */
void usage();

/**
 * @brief El manejador de señales
 */
void handle_signal(int sig);

void *handle_client(void *arg);

// Conexiones con el servidor
struct server_connection_node {
    pthread_t *threads;
    int sockets;
    struct server_connection_node *next;
} connection_head;

// Argumentos del manejador de clientes
struct clienth_args {
    int socket;
    pthread_t thread;
};

int lserver_socket;  // socket del servidor

int main(const int argc, const char *argv[]) {
    if (argc != 2) {
        usage();
        exit(EXIT_FAILURE);
    }
    // Argumentos de consola
    int port = atoi(argv[1]);

    struct stat vfile_stat;

    // Crear el directorio ".versions/" si no existe
#ifdef __linux__
    mkdir(VERSIONS_DIR, 0755);
#elif _WIN32
    mkdir(VERSIONS_DIR);
#endif

    // Crea el archivo .versions/versions.db si no existe
    if (stat(VERSIONS_DB_PATH, &vfile_stat) != 0) {
        creat(VERSIONS_DB_PATH, 0755);
    }

    int c;  // socket del cliente
    uint clilen;
    char buf[BUFSZ];  // buffer de la comunicación continua
    struct sockaddr_in addr, client_addr;

    // 0. instalar los manejadores de SIGINT, SIGTERM
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // 1. Obtener un conector
    lserver_socket = socket(AF_INET, SOCK_STREAM, 0);

    // 2. Asociar una direccion a un conector - bind
    memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    // TODO: debe de ser recibido por linea de comandos
    addr.sin_port = htons(port);

    addr.sin_addr.s_addr = INADDR_ANY;  // -> 0.0.0.0

    if (bind(lserver_socket, (struct sockaddr *)&addr,
             sizeof(struct sockaddr_in))) {
        perror("Binding error\n");
        exit(EXIT_FAILURE);
    }

    // 3. Colocar el socket disponible - listen
    if (listen(lserver_socket, 1)) {
        perror("Listening error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        pthread_t client_thread;
        puts("Waiting for client...");
        // 4. (bloqueante) Esperar por un cliente `c` - accept
        c = accept(lserver_socket, (struct sockaddr *)&client_addr, &clilen);
        if (c == -1) {
            perror("Error accepting client");
            continue;
        }
        if (pthread_create(&client_thread, NULL, handle_client, &c)) {
            perror("Error creating client thread");
            break;
        }
    }
    
    // 7. cerrar el socket del servidor lserver_socket
    close(lserver_socket);
    exit(EXIT_SUCCESS);
}

void usage() { puts("TODO: make usage"); }

void handle_signal(int sig) {
    if (sig == SIGTERM) {
        puts("Closed by terminal signal");
    }
    if (sig == SIGINT) {
        puts("Closed by keyboard");
    }
    close(lserver_socket);

    puts("Closing...");
    exit(EXIT_FAILURE);
}

void *handle_client(void *arg) {
    int c;  // socket del cliente
    char buffer[BUFSZ];
    int readed;
    int rcode;

    c = *(int *)arg;

    if (send_greeting(c, 0) || receive_greeting(c, 0)) {
        perror("Error in salute protocol");
        close(c);
        return NULL;
    }

    while (rcode = server_receive_request(c), rcode == 1);

    printf("Client %d disconnected\n", c);

    close(c);

    return NULL;
}