/**
 * @file
 * @brief Comunication process implementation
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @copyright MIT License
 */
#include "comunication.h"

#include <stdio.h>
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
int socket_active = 1;

void keep_comunicating(int s, char *username) {
	pthread_t write_thread, read_tread;
	struct write_thread_args write_args;
	write_args.s = s;
	write_args.username = username;
	pthread_create(&write_thread, NULL, write_thread_start, &write_args);
	pthread_create(&read_tread, NULL, read_thread_start, &s);

	pthread_join(write_thread, NULL);
	pthread_cancel(read_tread);
}


static void* write_thread_start(void *arg) {
	char buf[BUFSZ];
	char message[BUFSZ];
	int s = *((int*) arg);
	struct write_thread_args arg_struct = *(struct write_thread_args*)arg;

	while (1) {
		memset(buf, 0, BUFSZ);
		memset(message, 0, BUFSZ);
		sprintf(message, "%s: ", arg_struct.username);
		scanf("%s", buf);
		strcat(message, buf);
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
	int s = *((int*) arg);
	while (socket_active) {
		memset(buf, 0, BUFSZ);
		if (read(s, buf, BUFSZ) == -1) {
			perror("Read failed");
			break;
		}
		puts(buf);
	}
	return 0;
}
