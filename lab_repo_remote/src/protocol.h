/**
 * @file
 * @brief Interfaz Protocolo de comunicación
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andrés Martinez Varón <jorgeandre@unicauca.edu.co>
 * @copyright MIT License
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#define BUFSZ 80

#define SERVER_DIR ".server/"


/**
 * @brief Envía un mensaje de saludo
 * @param s Socket al que se envia el mensaje
 * @return 0 al enviar correctamente, -1 al tener un error
 */
int send_greeting(int s);

/**
 * @brief Recibe un mensaje del saludo
 * @param s Socket del que se recibe el mensaje
 * @return 0 al recibir correctamente, -1 al tener un error
 */
int receive_greeting(int s);

int client_add();

int client_get();

int client_list();

int server_add();

int server_get();

int server_list();



#endif
