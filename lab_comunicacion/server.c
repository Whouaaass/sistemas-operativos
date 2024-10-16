/**
 * @file
 * @brief Programa Server
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
#include "protocol.h"

void helper();

int main(int argc, char const *argv[])
{
	if (argc != 2) {
		helper();
		exit(EXIT_FAILURE);
	}
	// Argumentos de consola
	int port = atoi(argv[1]);

    int s; // socket del servidor
    int c; // socket del cliente
    uint clilen;
    char buf[BUFSZ]; // buffer de la comunicación continua
    struct sockaddr_in addr;


    // TODO!!
    // 0. instalar los manejadores de SIGINT, SIGTERM

    // 1. Obtener un conector
    s = socket(AF_INET, SOCK_STREAM, 0);

    // 2. Asociar una direccion a un conector - bind
    memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234); //TODO !!! -> debe de ser recibido por linea de comandos !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    addr.sin_addr.s_addr = INADDR_ANY; // -> 0.0.0.0

    if (bind(s, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)))
    {
    	perror("Algo malo sucedio con el bind\n");
     	exit(EXIT_FAILURE);
    }

    // 3. Colocar el socket disponible - listen
    if (listen(s, 1)) {
    	perror("Algo malo sucedio con el listen\n");
     	exit(EXIT_FAILURE);
    }

    // 4. (bloqueante) Esperar por un cliente `c` - accept
    puts("Waiting for client...");
    c = accept(s, (struct sockaddr *) &addr, &clilen);

    // 5. (comunicación)
    //      recibir el saludo
    //      enviar el saludo
    //      ....
    if (send_greeting(c) || receive_greeting(c)) {
    	perror("Error en el protocolo");
    	exit(EXIT_FAILURE);
    }

    while (1) {
    	memset(buf, 0, BUFSZ);
    	if (read(c, buf, BUFSZ) == -1) {
	    	perror("Read failed");
	     	exit(EXIT_FAILURE);
	    }
	    puts(buf);

		memset(buf, 0, BUFSZ);
    	scanf("%s", buf);
     	if (strcmp(buf, "close") == 0) {
      		break;
      	}
	    if (write(c, buf, BUFSZ) == -1) {
	   		perror("Write failed");
	    	exit(EXIT_FAILURE);
	    }
    }
    // 6. cerrar el socket del cliente c
    close(c);
    // 7. cerrar el socket del servidor s
    close(s);
    exit(EXIT_SUCCESS);
}


void helper() {
	puts("TODO!!: help text");
}
