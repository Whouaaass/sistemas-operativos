/**
 * @file serverv.h
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andrés Martinez Varón <jorgeandre@unicauca.edu.co>
 * @brief Métodos del servidor
 * 
 * @copyright MIT License
 * 
 */
#ifndef SERVERV_H
#define SERVERV_H

#include "userauth.h"

/**
 * Estructura que guarda los datos de sesión de un usuario 
 */
typedef struct {
    char username[USERNAME_SIZE];
    char authenticated;
} user_session;


/**
 * @brief Gestiona las peticiones al servidor
 *
 * @param s socket del cliente
 * @param session donde se guarda el nombre del usuario
 * @return int 0 para para salir, 1 para continuar, -1 para error
 */
int server_receive_request(int s, user_session *session);

#endif