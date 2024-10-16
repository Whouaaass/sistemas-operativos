/**
 * @file
 * @brief Comunication process implementation
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @copyright MIT License
 */
#include "comunication.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "protocol.h"

static void* write_thread_start(void *arg);
static void* read_thread_start(void *arg);

/**
 * structure used in the thread for reading the socket
*/
struct write_thread_args {
	int s;
	char* username;
};

struct read_thread_args {
	int s;
	pthread_t write_thread;
};

int socket_active = 1;

void keep_comunicating(int s, char *username) {
	pthread_t write_thread, read_thread;
	struct write_thread_args write_args;
	struct read_thread_args read_args;

	write_args.s = s;
	write_args.username = username;
	pthread_create(&write_thread, NULL, write_thread_start, &write_args);

	read_args.s = s;
	read_args.write_thread = write_thread;
	pthread_create(&read_thread, NULL, read_thread_start, &read_args);

	pthread_join(write_thread, NULL);
	pthread_cancel(read_thread);
}


static void* write_thread_start(void *arg) {
	char buf[BUFSZ];
	char message[BUFSZ];
	int s = *((int*) arg);
	struct write_thread_args arg_struct = *(struct write_thread_args*)arg;

	while (socket_active) {
		memset(buf, 0, BUFSZ);
		memset(message, 0, BUFSZ);
		scanf("%s", buf);
		sprintf(message, "%s: %s", arg_struct.username, buf);
		if (strcmp(buf, "close") == 0) {
			break;
		}
		if (write(arg_struct.s, message, BUFSZ) == -1) {
			perror("Reading error");
			break;
		}

	}
	socket_active = 0;
	return 0;
}

static void* read_thread_start(void *arg) {
	char buf[BUFSZ];
	struct read_thread_args arg_struct = *(struct read_thread_args*)arg;
	while (socket_active) {
		memset(buf, 0, BUFSZ);
		ssize_t readed = read(arg_struct.s, buf, BUFSZ);
		if (readed == -1) {
			perror("Read failed");
			break;
		}
		if (readed == 0) {
			puts("closed connection");
			puts("Ingresa una tecla para salir");
			break;
		}
		puts(buf);
	}
	//pthread_cancel(arg_struct.write_thread);
	socket_active = 0;
	return 0;
}
