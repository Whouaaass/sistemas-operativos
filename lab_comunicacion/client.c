/**
 * @file
 * @brief Programa Cliente
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @copyright MIT License
 */
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include "comunication.h"
#include "protocol.h"

/**
* prints message on bad command usage
*/
void helper();

/**
* signal handlers
*/
void handle_sig(int sig);

int main(int argc, char const *argv[])
{
	if (argc != 3) {
		helper();
		exit(EXIT_FAILURE);
	}

	//Argumentos de consola
	const char *ip_addr = argv[1]; // ip address
	const int port = atoi(argv[2]); // port

	int s; // socket del servidor
    uint clilen;
    char buf[BUFSZ];
    struct sockaddr_in addr;

    // 0. instalar los manejadores de SIGINT, SIGTERM
    signal(SIGINT, handle_sig);
    signal(SIGTERM, handle_sig);


    // 1. obtener un conector - socket
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    	perror("Error al crear el socket");
     	exit(EXIT_FAILURE);
    }

    // 2. conectarse a una dirección (IP, puerto) - connect ~ bind
    memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    if (inet_aton(ip_addr, &addr.sin_addr) == 0) {
    	perror("Error initializing address");
    	exit(EXIT_FAILURE);
    }
    addr.sin_port = htons(port);

    puts("Connecting...");
    if (connect(s, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)))
    {
    	perror("Binding error");
     	exit(EXIT_FAILURE);
    }

    // 3. (comunicación)
    //      enviar un saludo - write
    //      recibir un saludo - receive
    //      ...
    if (send_greeting(s) || receive_greeting(s)) {
    	perror("Error in salute protocol");
     	exit(EXIT_FAILURE);
    }

    keep_comunicating(s, "client");

    // 4. cerrar la conexión con el servidor - close
    close(s);
    exit(EXIT_SUCCESS);
}

void helper() {
	puts("Usage: client ADDRESS PORT");
	puts("");
}

void handle_sig(int sig)
{
    printf("Closing... %d\n", sig);
    exit(EXIT_FAILURE);
}
