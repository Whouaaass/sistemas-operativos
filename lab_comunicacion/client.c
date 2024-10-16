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
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include "protocol.h"

void helper();

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
    char* buf;
    struct sockaddr_in addr;

    // 0. instalar los manejadores de SIGINT, SIGTERM

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
    	perror("Algo malo sucedio con el bind");
     	exit(EXIT_FAILURE);
    }

    // 3. (comunicación)
    //      enviar un saludo - write
    //      recibir un saludo - receive
    //      ...
    if (send_greeting(s) || receive_greeting(s)) {
    	perror("Error en el protocolo");
     	exit(EXIT_FAILURE);
    }

    while (1) {
    	scanf("%s", buf);
   		if (strcmp(buf, "close") == 0) {
    		break;
    	}
	    if (write(s, buf, BUFSZ) == -1) {
	   		perror("Write failed");
	    	exit(EXIT_FAILURE);
	    }
	   	memset(buf, 0, BUFSZ);
	   	if (read(s, buf, BUFSZ) == -1) {
	    	perror("Read failed");
	     	exit(EXIT_FAILURE);
	    }
	    puts(buf);
    }

    // 4. cerrar la conexión con el servidor - close
    close(s);
    exit(EXIT_FAILURE);
}

void helper() {
	puts("TODO!!: command help");
}
