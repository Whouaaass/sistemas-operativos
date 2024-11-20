/**
 * @file clientv.h
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andrés Martinez Varón <jorgeandre@unicauca.edu.co>
 * @brief Métodos del cliente 
 * 
 * @copyright MIT License 
 */

#ifndef CLIENTV_H
#define CLIENTV_H

#include <pthread.h>

#include "protocol.h"

/**
 * @brief Ejecuta el protocolo del cliente en el método add
 * @param s socket del server
 * @param filename nombre del archivo
 * @param comment comentario del guardado
 * @return el codigo de retorno del server en cualquiera de las etapass
 */
pres_code client_add(int s, char* filename, char* comment);

/**
 * @brief Ejecuta el protocolo del cliente en el método get
 *
 * @param s socket del server
 * @param filename nombre del archivo
 * @param version versión del archivo
 * @return sres_code
 */
pres_code client_get(int s, char* filename, int version);

/**
 * @brief Ejecuta el protocolo del cliente en el método list
 *
 * @param s socket del server
 * @param filename nombre del archivo
 * @return sres_code
 */
pres_code client_list(int s, char* filename);

/**
 * @brief autentifica esta sesión
 * 
 * @param s socket con el servidor
 * @return int 0 para caso de exito, -1 para error
 */
int authenticate_session(int s, char* username, char* password);

/**
 * @brief crea a un usuario
 * 
 * @param s socket con el servidor
 * @param username nombre de usuario
 * @param password contraseña de ese usuario
 * @return int 0 para caso de exito, -1 para error
 */
int register_user(int s, char* username, char* password);



#endif