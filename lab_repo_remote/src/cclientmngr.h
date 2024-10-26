/**
 * @file cclientmngr.h
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andrés Martinez Varón <jorgeandre@unicauca.edu.co>
 * @brief Es el gestor de clientes, se encarga de administrar los sockets de los
 * clientes*
 * @copyright MIT License
 */

#ifndef CCLIENTMNGR_H
#define CCLIENTMNGR_H

#include <pthread.h>

#include "protocol.h"

// Esto estaba pensado para que no dara error de binding en el socket
// pero al final no arregla el problema porque es estricto que el socket se cierre desde el
// cliente primero

/**
 * @brief Inicializa el manejador de clientes
 * Ahora mismo esto es necesario por el mutex
 */
void init_cclient_manager();

/**
 * @brief Añade un cliente a la lista de clientes conectados
 *
 * @param socket socket del cliente
 */
void add_cclient(int socket);

/**
 * @brief Elimina un cliente (cerrando su conexión)
 *
 * @param socket socket del cliente
 */
void dismiss_cclient(int socket);

/**
 * @brief Cierra la conexión con todos los clientes
 */
void dismiss_all_cclients();

#endif
