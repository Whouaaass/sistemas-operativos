/**
 * @file
 * @brief Sistema de control de versiones CLIENTE
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andrés Martinez Varón <jorgeandre@unicauca.edu.co>
 * @copyright MIT License
 */
#include <arpa/inet.h>
#include <libgen.h>
#include <linux/limits.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "clientv.h"
#include "strprocessor.h"
#include "versions.h"

int server_socket;  // socket con el servidor

/**
 * @brief Crea la conección con el servidor
 *
 * @param ip dirección IPv4 del servidor
 * @param port puerto del servidor
 * @return int the socket con el servidor, -1 para errores
 */
int make_connection(char *ip, int port);

/**
 * @brief Maneja los comandos del programa
 *
 * @param s socket con el servidor
 */
void manage_commands(int s);

/**
 * @brief Imprime el mensaje de ayuda
 */
void usage();

/**
 * @brief Imprime el mensaje de ayuda para los comandos internos
 *
 */
void inner_usage();

/**
 * @brief Imprime el mensaje de error de un método de la API
 * 
 * @param code codigo de error
 */
void print_serror_message(sres_code code);

/**
 * @brief El manejador de señales
 * @param sig numero de señal
 */
void handle_signal(int sig);

/**
 * @brief Funcion usada para terminar el programa
 * @param exit_code codigo de salida del programa
 */
void terminate(int exit_code);

int main(int argc, char *argv[]) {
    int rcode = -1;  // codigo de retorno
    char *filename;  // nombre del archivo

    int arg_port;  // puerto del servidor
    char *arg_ip;  // ip del servidor

    // 1. instalar los manejadores de SIGINT, SIGTERM
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // 2. maneja los argumentos del programa
    if (argc == 3) {
        arg_port = atoi(argv[2]);
        arg_ip = argv[1];
        if (arg_port < 0 || arg_port > 65535) {
            printf("El puerto debe estar entre 0 y 65535\n");
            exit(EXIT_FAILURE);
        }

        server_socket = make_connection(arg_ip, arg_port);
        if (server_socket == -1) {
            perror("Error connecting to server");
            exit(EXIT_FAILURE);
        }
        manage_commands(server_socket);
    } else if (argc == 2 &&
               (EQUALS(argv[1], "-h") || EQUALS(argv[1], "--help"))) {
        usage();
        exit(EXIT_SUCCESS);
    } else {
        usage();
        exit(EXIT_FAILURE);
    }

    terminate(EXIT_SUCCESS);
}

int make_connection(char *ip, int port) {
    int s;  // socket del servidor
    struct sockaddr_in addr;

    // 1. obtener un conector - socket
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) return -1;

    // 2. conectarse a una dirección (IP, puerto) - connect ~ bind
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_aton(ip, &addr.sin_addr) == 0) return 1;

    puts("Connecting to server...");
    if (connect(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_in))) {
        return -1;
    }
    printf("Connected to %s!\n", ip);

    // 3. Hacer el protocolo del saludo
    if (send_greeting(s, 1) == -1) {
        return -1;
    }
    if (receive_greeting(s, 1) == -1) {
        return -1;
    }

    return s;
}

void manage_commands(int s) {
    char **argv;
    int argc;
    int rcode;
    int readed;
    char stdin_buf[BUFSIZ];
    inner_usage();

    while (1) {
        printf("rversions> ");
        memset(stdin_buf, 0, BUFSIZ);
        if (fgets(stdin_buf, BUFSIZ, stdin) == NULL) break;
        readed = strlen(stdin_buf);
        if (stdin_buf[readed - 1] == '\n') stdin_buf[readed - 1] = 0;
        argv = split_commandline(stdin_buf, &argc);        

        if (EQUALS(argv[0], "exit")) {
            terminate(EXIT_SUCCESS);
        } else if (EQUALS(argv[0], "list") && argc == 1) {
            rcode = client_list(s, NULL);
        } else if (EQUALS(argv[0], "list") && argc == 2) {
            puts("list");
            rcode = client_list(s, argv[1]);
        } else if (EQUALS(argv[0], "add") && argc == 3) {
            rcode = client_add(s, argv[1], argv[2]);
        } else if (EQUALS(argv[0], "get") && argc == 3) {
            int version = atoi(argv[1]);
            rcode = client_get(s, argv[2], version);
        } else if (EQUALS(argv[0], "help")) {
            inner_usage();
        } else {
            printf("Invalid command\n");
        }
        free(argv);
        if (rcode != RSERVER_OK) print_serror_message(rcode);
    }
}

void usage() { puts("usage: rversions PORT IP"); }

void inner_usage() {
    puts(
        "commands:\n"
        "\tlist [filename]       Lista los archivos almacenados en el "
        "repositorio\n"
        "\tadd filename comment  Añade un archivo con el comentario a un "
        "servidor remoto\n"
        "\tget version filename  Obtiene el archivo especificado\n"
        "\thelp                  Imprime la ayuda\n"
        "\texit                  Termina el programa");
}

void print_serror_message(sres_code code) {
    switch (code) {
        case RFILE_TO_DATE:
            puts("El archivo ya está actualizado");
            break;
        case RFILE_OUTDATED:
            puts("El archivo no esta actualizado");
            break;
        case RFILE_NOT_FOUND:
            puts("El archivo no existe");
            break;
        case RVERSION_NOT_FOUND:
            puts("La versión solicitada no existe");
            break;
        case RSOCKET_ERROR:
            puts("Error de socket (En escritura o lectura)");
            break;
        case RILLEGAL_METHOD:
            puts("Método no permitido");
            break;
        case RERROR:
            perror("Error no especificado");
            break;
        default:
            perror("Error desconocido");
            break;
    }
}

void handle_signal(int sig) {
    if (sig == SIGTERM) {
        puts("Closed by terminal signal");
    }
    if (sig == SIGINT) {
        printf("\n");
        puts("Closed by keyboard");
    }
    terminate(EXIT_FAILURE);
}

void terminate(int exit_code) {
    puts("Ending program...");
    shutdown(server_socket, SHUT_RDWR);
    exit(exit_code);
}
