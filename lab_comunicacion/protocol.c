/**
 * @file
 * @brief Protocolo de comunicación
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @copyright MIT License
 */
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "protocol.h"

int send_greeting(int s) {
	char buf[BUFSZ];
	strcpy(buf, "Hola");
	if (write(s, buf, BUFSZ) == -1) {
		perror("Write failed");
		return -1;
	}
	return 0;
}

int receive_greeting(int s) {
	char buf[BUFSZ];
	memset(buf, 0, BUFSZ);
	if (read(s, buf, BUFSZ) == -1) {
		perror("Read failed");
		return -1;
	}
	return 0;
}
