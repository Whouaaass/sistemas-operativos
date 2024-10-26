/**
 * @file
 * @brief Interfaz Protocolo de comunicación
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andrés Martinez Varón <jorgeandre@unicauca.edu.co>
 * @copyright MIT License
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <limits.h>
#include <stdint.h>

#include "versions.h"

/* Tamaño del buffer para mensajers simples al socket */
#define BUFSZ 80
/* Tamaño de la longuitud del mensaje de longuitud de un archivo */
#define content_size uint32_t
/* Tamaño máximo de la logitud del contenido del mensaje */
#define content_max UINT32_MAX

/**
 * Codigo de los métodos
 */
typedef enum { GET, ADD, LIST, EXIT } method_code;

/**
 * Codigo de las respuestas del servidor
 */
typedef enum {
    RSERVER_OK,         /* !< Petición procesada correctamente */
    RFILE_TO_DATE,      /* !< El archivo mandado está actualizado */
    RFILE_OUTDATED,     /* !< El archivo mandado está desactualizado */
    RFILE_NOT_FOUND,    /* !< El archivo no existe */
    RVERSION_NOT_FOUND, /* !< La versión solicitada no existe */
    RSOCKET_ERROR,      /* !< Error de socket (En escritura o lectura) */
    RILLEGAL_METHOD,    /* !< Método no permitido */
    RERROR,             /* !< Error no especificado */
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
 * @brief Envía un archivo al socket de destino
 *
 * @param s socket de destino
 * @param filepath ruta donde se encuentra el archivo
 * @return int 0 en caso de exito, -1 en caso de error
 */
int send_file(int s, char* filepath);

/**
 * @brief Recibe un archivo del socket
 *
 * @param s socket del que se recibe el archivo
 * @param filepath ruta en la que se guardará el archivo
 * @return int 0 en caso de exito, -1 en caso de error
 */
int receive_file(int s, char* filepath);

/**
 * @brief Manda una cadena al servidor
 * en este caso primero se envia el tamaño de la cadena, para después
 * enviar el contenido de la cadena.
 *
 * @param s socket al cual enviar el mensaje
 * @param str cadena a enviar
 * @return int codigo de éxito (0) o error (-1)
 */
int send_string(int s, char *str);

/**
 * @brief Recibe una cadena del servidor
 * en este caso primero se recibe el tamaño de la cadena, para después
 * recibir el contenido de la cadena.
 *
 * @param s socket del cual recibir la cadena
 * @param str en donde se almacenara la cadena (si la cadena no es
 * suficientemente larga, se ignoran los caracteres)
 * @param max_size tamaño máximo de la cadena
 * @return int codigo de éxito (0) o error (-1)
 */
int receive_string(int s, char *str, size_t max_size);

/**
 * @brief Manda un mensaje al servidor
 * Este mensaje pretende enviar cualquier tipo de estructura cuando se
 * sabe de las dos partes el tamaño de esta estructura.
 *
 * @param s socket al cual enviar el mensaje
 * @param data el mensaje a enviar
 * @param size tamaño en bytes del mensaje
 * @return int codigo de éxito (0) o error (-1)
 */
int send_data(int s, void *data, size_t size);

/**
 * @brief Recibe un mensaje del servidor
 * Este mensaje pretende recibir cualquier tipo de estructura cuando se
 * sabe de las dos partes el tamaño de esta estructura.
 *
 * @param s socket al cual enviar el mensaje
 * @param data en donde se almacenara el mensaje
 * @param size tamaño en bytes del mensaje
 * @return int codigo de éxito (0) o error (-1)
 */
int receive_data(int s, void *data, size_t size);


#endif
