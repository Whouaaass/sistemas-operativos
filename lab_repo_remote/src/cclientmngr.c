/**
 * @file cclientmngr.c
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andrés Martinez Varón <jorgeandre@unicauca.edu.co>
 * @brief Implementación del gestor de clientes
 * @version 0.1
 * @date 2024-10-25
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "cclientmngr.h"

#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <sys/socket.h>

pthread_mutex_t clientl_lock; /* Mutex para la lista de clientes */

/* Lista anidada que contiene los sockets de los clientes */
struct client_node {
    int socket;
    struct client_node *next;
};

/* Puntero a la cabeza de la lista */
struct client_node *client_head = NULL;

/**
 * @brief Despide al cliente (no lo borra de la lista, solo lo elimina y cierra)
 * 
 * @param client cliente a despedir
 */
void dismiss_client(struct client_node *client);

void init_cclient_manager() {
    if (pthread_mutex_init(&clientl_lock, NULL)) {
        perror("Error initializing client mutex");
        exit(EXIT_FAILURE);
    }
    client_head = NULL;
}

void add_cclient(int socket) {
    pthread_mutex_lock(&clientl_lock);
    struct client_node *head = client_head;

    head = calloc(0, sizeof(struct client_node));
    head->socket = socket;
    head->next = client_head;
    client_head = head;

    pthread_mutex_unlock(&clientl_lock);
}

void dismiss_cclient(int socket) {
    pthread_mutex_lock(&clientl_lock);

    struct client_node **head = &client_head;

    while (*head != NULL) {
        if ((*head)->socket == socket) {
            struct client_node *tmp = *head;
            *head = (*head)->next;
            dismiss_client(tmp);
            break;
        }
        head = &(*head)->next;
    }

    pthread_mutex_unlock(&clientl_lock);
}

void dismiss_all_cclients() {
    pthread_mutex_lock(&clientl_lock);

    struct client_node *head = client_head;
    while (head != NULL) {        
        struct client_node *tmp = head;
        printf("Despidiendo cliente con socket %d\n", tmp->socket);
        head = head->next;
        dismiss_client(tmp);
    }

    client_head = NULL;
    pthread_mutex_unlock(&clientl_lock);
}


void dismiss_client(struct client_node *client) {
    printf("Despidiendo cliente con socket %d\n", client->socket);
    int r_code = shutdown(client->socket, SHUT_RDWR);
    if (r_code == -1) {
        perror("Error closing socket");
    }
    free(client);
}