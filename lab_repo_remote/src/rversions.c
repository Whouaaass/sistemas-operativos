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
#include "protocol.h"
#include "strprocessor.h"
#include "userauth.h"
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
 * @brief Obtiene las credenciales del cliente
 *
 * @param username cadena en la que se almacenará el nombre del usuario
 * @param password cadena en la que se almacenará la contraseña
 * @return int 0 para caso de exito, -1 para error
 */
int get_client_credentials(char *username, char *password);

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
    char *username;
    char *password;

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
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];

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
    
    int readed;
    char stdin_buf[BUFSIZ];
    char username[USERNAME_SIZE], password[PASSWORD_SIZE];
    inner_usage();

    while (1) {
        pres_code rcode = RSERVER_OK;
        printf("rversions> ");
        memset(stdin_buf, 0, BUFSIZ);
        if (fgets(stdin_buf, BUFSIZ, stdin) == NULL) break;
        readed = strlen(stdin_buf);        
        if (stdin_buf[readed - 1] == '\n') stdin_buf[readed - 1] = 0;
        argv = split_commandline(stdin_buf, &argc);

        if (argv == NULL) continue; 
        

        if (argc == 1 && EQUALS(argv[0], "exit")) {
            terminate(EXIT_SUCCESS);
        } else if (argc == 1 && EQUALS(argv[0], "login")) {
            puts("Iniciando sesión...");
            get_client_credentials(username, password);
            rcode = authenticate_session(s, username, password);
        } else if (argc == 1 && EQUALS(argv[0], "register")) {
            puts("Registrandose...");
            get_client_credentials(username, password);
            rcode = register_user(s, username, password);
        } else if (argc == 1 && EQUALS(argv[0], "list")) {
            rcode = client_list(s, NULL);
        } else if (argc == 2 && EQUALS(argv[0], "list")) {            
            rcode = client_list(s, argv[1]);
        } else if (argc == 3 && EQUALS(argv[0], "add")) {
            rcode = client_add(s, argv[1], argv[2]);
        } else if (argc == 3 && EQUALS(argv[0], "get")) {
            int version = atoi(argv[1]);
            rcode = client_get(s, argv[2], version);
        } else if (argc == 1 && EQUALS(argv[0], "help")) {
            inner_usage();
        } else {
            printf("Invalid command\n");
        }
        for (int i = 0; i < argc; i++) free(argv[i]);
        free(argv);
        if (rcode != RSERVER_OK) puts(get_protocol_rmsg(rcode));
    }
}

void usage() { puts("usage: rversions PORT IP"); }

void inner_usage() {
    puts(
        "commands:\n"
        "\tlogin Autentica al usuario\n"
        "\tregister Registra un usuario\n"
        "\tlist [filename]       Lista los archivos almacenados en el "
        "repositorio\n"
        "\tadd filename comment  Añade un archivo con el comentario a un "
        "servidor remoto\n"
        "\tget version filename  Obtiene el archivo especificado\n"
        "\thelp                  Imprime la ayuda\n"
        "\texit                  Termina el programa");
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

int get_client_credentials(char *username, char *password) {
    const char *username_prompt = "Usuario: ";
    const char *password_prompt = "Contraseña: ";
    char *pass;
    size_t readed;

    printf("%s", username_prompt);
    fgets(username, USERNAME_SIZE, stdin);
    username[strlen(username) - 1] = 0;

    pass = getpass(password_prompt);

    memcpy(password, pass, PASSWORD_SIZE);
    return 0;
}