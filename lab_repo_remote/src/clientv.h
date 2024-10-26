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
sres_code client_add(int s, char* filename, char* comment);

/**
 * @brief Ejecuta el protocolo del cliente en el método get
 *
 * @param s socket del server
 * @param filename nombre del archivo
 * @param version versión del archivo
 * @return sres_code
 */
sres_code client_get(int s, char* filename, int version);

/**
 * @brief Ejecuta el protocolo del cliente en el método list
 *
 * @param s socket del server
 * @param filename nombre del archivo
 * @return sres_code
 */
sres_code client_list(int s, char* filename);


#endif