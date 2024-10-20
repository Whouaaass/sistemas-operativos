/**
 * @file
 * @brief API de gestion de versiones
 * @author Erwin Meza Vega <emezav@unicauca.edu.co>
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andres Martinez Varon <jorgeandre@unicauca.edu.co>
 * @copyright MIT License
 */

#ifndef VERSIONS_H
#define VERSIONS_H

#ifdef __linux__
#include <linux/limits.h>
#endif

#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "sha256.h"

#define COMMENT_SIZE 80 /** < Longitud del comentario */
#define HASH_SIZE 256   /**< Longitud del hash incluyendo NULL*/
#define READ_CHUNK PATH_MAX

#define VERSIONS_DB \
    "versions.db" /**< Nombre de la base de datos de versiones. */
#define VERSIONS_DIR ".versions" /**< Directorio del repositorio. */
#define VERSIONS_DB_PATH \
    VERSIONS_DIR "/" VERSIONS_DB /**< Ruta completa de la base de datos.*/

#define EQUALS(s1, s2) \
    (strcmp(s1, s2) == 0) /**< Verdadero si dos cadenas son iguales.*/

/**
 * @brief Version de un archivo.
 * Para cada version de un archivo se almacena el nombre original,
 * el comentario del usuario y el hash de su contenido.
 * El hash es a la vez el nombre del archivo dentro del
 * repositorio.
 */
typedef struct __attribute__((aligned(512))) {
    char filename[PATH_MAX];    /**< Nombre del archivo original. */
    char hash[HASH_SIZE];       /**< Hash del contenido del archivo. */
    char comment[COMMENT_SIZE]; /**< Comentario del usuario. */
} file_version;

/**
 * @brief Codigo de retorno de operacion
 */
typedef enum {
    VERSION_ERROR,          /*!< Error no especificado */
    VERSION_CREATED,        /*!< Version creada */
    VERSION_ADDED,          /*!< Version agregada */
    VERSION_OK,             /*!< La operacion se completo de forma exitosa */
    VERSION_ALREADY_EXISTS, /*!< Version ya existe */
    VERSION_NOT_FOUND,      /*!< La Version buscada no existe */
    FILE_ADDED,             /*!< Archivo adicionado  */

    /* .. */
} return_code;

/**
 * @brief Adiciona un archivo al repositorio.
 *
 * @param filename Nombre del archivo a adicionar
 * @param comment Comentario de la version actual
 *
 * @return Codigo de la operacion
 */
return_code add(char *filename, char *comment);

/**
 * @brief Lista las versiones de un archivo.
 *
 * @param filename Nombre del archivo, NULL para listar todo el repositorio.
 */
void list(char *filename);

/**
 * @brief Obtiene una version del un archivo.
 * Sobreescribe la version existente.
 *
 * @param filename Nombre de archivo.
 * @param version Numero secuencial de la version.
 *
 * @return 1 en caso de exito, 0 si ocurre un error.
 */
return_code get(char *filename, int version);

/**
 * @brief Crea una version en memoria del archivo
 * Valida si el archivo especificado existe y crea su hash
 *
 * @param filename nombre del archivo
 * @param comment comentario
 * @param result resultado
 * @return return_code
 */
return_code create_version(char *filename, char *comment, file_version *result);

/**
 * @brief guarda una version en el repositori
 * @param v version de archivo que se guardará
 * @return return_code
 */
return_code add_new_version(file_version *v);

/**
 * @brief get_file_hash
 * @param filename nombre del archivo
 * @param hash buffer para almacenar el hash
 * @return char*
 */
char *get_file_hash(char *filename, char *hash);

/**
 * @brief imprime la estructura que guarda la version
 * @param v version de archivo que se imprimirá
 */
void print_version(const file_version *v);

/**
 * @brief indica si existe una version para un archivo
 *
 */
int version_exists(char *filename, char *hash);

/**
 * @brief Obtiene una version del un archivo.
 * 
 * @param v version del archivo solicitada
 * @param filename nombre del archivo del cual se busca la version
 * @param version version del archivo
 * @return int 
 */
int get_version(file_version *v, char *filename, int version);

#endif