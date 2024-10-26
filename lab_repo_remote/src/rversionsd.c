/**
 * @file
 * @brief Sistema de control de versiones SERVER
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andrés Martinez Varón <jorgeandre@unicauca.edu.co>
 * @copyright MIT License
 */

#include <arpa/inet.h>
#include <bits/pthreadtypes.h>
#include <errno.h>
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

#include "cclientmngr.h"
#include "protocol.h"

#define max_clients 2

void print_socket_error(int err) {
    switch (err) {
        case EACCES:
            printf("EACCES: ");
            printf(
                "For UNIX domain sockets, write permission is denied on the "
                "socket file, "
                "or search permission is denied for one of the directories in "
                "the path prefix.\n");
            break;
        case EPERM:
            printf("EPERM: ");
            printf(
                "The user tried to connect to a broadcast address without "
                "having the socket broadcast "
                "flag enabled or the connection request failed because of a "
                "local firewall rule.\n");
            break;
        case EADDRINUSE:
            printf("EADDRINUSE: Local address is already in use.\n");
            break;
        case EADDRNOTAVAIL:
            printf(
                "EADDRNOTAVAIL: The socket had not been bound to an address "
                "and all ephemeral ports are in use.\n");
            break;
        case EAFNOSUPPORT:
            printf(
                "EAFNOSUPPORT: The passed address didn't have the correct "
                "address family.\n");
            break;
        case EAGAIN:
            printf(
                "EAGAIN: For nonblocking sockets, the connection cannot be "
                "completed immediately.\n");
            break;
        case EALREADY:
            printf(
                "EALREADY: The socket is nonblocking and a previous connection "
                "attempt is not yet completed.\n");
            break;
        case EBADF:
            printf("EBADF: sockfd is not a valid open file descriptor.\n");
            break;
        case ECONNREFUSED:
            printf("ECONNREFUSED: No one listening on the remote address.\n");
            break;
        case EFAULT:
            printf(
                "EFAULT: The socket structure address is outside the user's "
                "address space.\n");
            break;
        case EINPROGRESS:
            printf(
                "EINPROGRESS: The connection cannot be completed "
                "immediately.\n");
            break;
        case EINTR:
            printf(
                "EINTR: The system call was interrupted by a caught signal.\n");
            break;
        case EISCONN:
            printf("EISCONN: The socket is already connected.\n");
            break;
        case ENETUNREACH:
            printf("ENETUNREACH: Network is unreachable.\n");
            break;
        case ENOTSOCK:
            printf(
                "ENOTSOCK: The file descriptor does not refer to a socket.\n");
            break;
        case EPROTOTYPE:
            printf(
                "EPROTOTYPE: The socket type does not support the requested "
                "communications protocol.\n");
            break;
        case ETIMEDOUT:
            printf(
                "ETIMEDOUT: Timeout while attempting connection; server may be "
                "too busy.\n");
            break;
        default:
            printf("Unknown error %d: %s\n", err, strerror(err));
    }
}

/**
 * @brief Imprime el mensaje de ayuda
 */
void usage();

/**
 * @brief El terminador del programa
 * @param sig retorno del programa
 */
void terminate(int sig);

/**
 * @brief El manejador de señales
 */
void handle_signal(int sig);

/**
 * @brief El manejador de un hilo de un cliente
 *
 * @param arg argumentos del cliente
 */
void *handle_client(void *arg);

/* Argumentos del manejador de clientes */
struct clienth_args {
    int socket;
    pthread_t thread;
};

int lserver_socket; /* socket del servidor */

int main(const int argc, const char *argv[]) {
    if (argc != 2) {
        usage();
        exit(EXIT_FAILURE);
    }
    // Argumentos de consola
    int port = atoi(argv[1]);

    struct stat vfile_stat; /* Estado del archivo versions.db */

    // Crear el directorio ".versions/" si no existe
#ifdef __linux__
    mkdir(VERSIONS_DIR, 0755);
#elif _WIN32
    mkdir(VERSIONS_DIR);
#endif

    // Inicializa el manejador de clientes
    init_cclient_manager();

    // Crea el archivo .versions/versions.db si no existe
    if (stat(VERSIONS_DB_PATH, &vfile_stat) != 0) {
        creat(VERSIONS_DB_PATH, 0755);
    }

    int c; /* socket del cliente */
    struct sockaddr_in addr;

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
    if (listen(lserver_socket, 2)) {
        perror("Listening error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        socklen_t clilen = sizeof(struct sockaddr_in);
        struct sockaddr_in client_addr;
        pthread_t client_thread;

        puts("Waiting for client...");
        // 4. (bloqueante) Esperar por un cliente `c` - accept
        c = accept(lserver_socket, (struct sockaddr *)&client_addr, &clilen);
        if (c == -1) {
            perror("Error accepting client");
            if (errno == EADDRNOTAVAIL) puts("Interrupted by signal");
            print_socket_error(errno);
            continue;
        }
        add_cclient(c);
        if (pthread_create(&client_thread, NULL, handle_client, &c)) {
            perror("Error creating client thread");
            break;
        }
    }

    // 7. cerrar el socket del servidor lserver_socket
    terminate(EXIT_SUCCESS);
}

void usage() {
    puts(
        "usage: rversionsd PORT"
        "\tPORT: puerto del servidor");
}

void handle_signal(int sig) {
    if (sig == SIGTERM) puts("Closed by terminal signal");
    if (sig == SIGINT) puts("\nClosed by keyboard");
    terminate(sig);
}

void *handle_client(void *arg) {
    int c;  // socket del cliente
    char buffer[BUFSZ];
    int readed;
    int rcode;

    c = *(int *)arg;

    if (send_greeting(c, 0) || receive_greeting(c, 0)) {
        perror("Error in salute protocol");
        dismiss_cclient(c);
        return NULL;
    }

    // Mantiene la conexión con el cliente activa hasta que se indique lo
    // contrario
    while (rcode = server_receive_request(c), rcode == 1);

    printf("Client %d disconnected\n", c);

    dismiss_cclient(c);

    return NULL;
}

void terminate(int sig) {
    puts("Closing...");

    close(lserver_socket);
    dismiss_all_cclients();

    exit(EXIT_FAILURE);
}