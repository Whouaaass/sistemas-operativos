/**
 * @file
 * @brief Interfaz Protocolo de comunicación
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andrés Martinez Varón <jorgeandre@unicauca.edu.co>
 * @copyright MIT License
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <limits.h>
#include "versions.h"

#define BUFSZ 80 /* !< Tamaño del buffer para mensajers simples al socket*/

#define content_size                                                      \
    uint32_t /* Tipo para definir la longuitud del tamaño del mensaje de \
                longuitud de un archivo */
#define content_max UINT32_MAX /* !< Tamaño máximo del contenido del mensaje */
/**
 * Codigo de los métodos
 */
typedef enum { GET, ADD, LIST } method_code;

/**
 * Codigo de las respuestas del servidor
 */
typedef enum {
    RSERVER_OK, /* !< Petición procesada correctamente */
    RFILE_TO_DATE, /* !< El archivo mandado está actualizado */
    RFILE_OUTDATED, /* !< El archivo mandado está desactualizado */
    RFILE_NOT_FOUND, /* !< El archivo no existe */
    RVERSION_NOT_FOUND, /* !< La versión solicitada no existe */
    RSOCKET_ERROR, /* !< Error de socket (En escritura o lectura) */    
    RILLEGAL_METHOD, /* !< Método no permitido */
    RERROR, /* !< Error no especificado */    
} sres_code;

/**
 * Codigo de las respuestas del cliente
 */
typedef enum { CONFIRM, DENY, END } cres_code;

/**
 * Estructura de peticion de get
 */
struct get_request {    
    char filename[PATH_MAX];
    int version;
};

/**
 * Estructura de peticion de add 
 */
struct add_request {
    char filename[PATH_MAX];
    char hash[HASH_SIZE];    
    char comment[COMMENT_SIZE];
};


/**
 * @brief Envía un mensaje de saludo
 * @param s Socket al que se envia el mensaje
 * @param greeter Indica si es el que saludo primero (1: para ser el greeter, 0:
 * para ser el el greeted)
 * @return 0 al enviar correctamente, -1 al tener un error
 */
int send_greeting(int s, const int greeter);

/**
 * @brief Recibe un mensaje del saludo
 * @param s Socket del que se recibe el mensaje
 * @param greeter Indica si es el que saludo primero
 * @return 0 al recibir correctamente, -1 al tener un error
 */
int receive_greeting(int s, const int greeter);

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

/**
 * @brief Gestiona las peticiones al servidor
 * 
 * @param s socket del cliente
 * @return int 0 para exito, -1 para error
 */
sres_code server_receive_request(int s);

#endif
