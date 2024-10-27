/**
 * @file versions.h
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

/** Longitud del comentario */
#define COMMENT_SIZE 80
/** Longitud del hash incluyendo NULL*/
#define HASH_SIZE 256
/* TAmaño de leida de un archivo */
#define READ_CHUNK PATH_MAX
/** Nombre de la base de datos de versiones. */
#define VERSIONS_DB "versions.db"
/** Directorio del repositorio. */
#define VERSIONS_DIR "files"
/** Ruta completa de la base de datos.*/
#define VERSIONS_DB_PATH VERSIONS_DIR "/" VERSIONS_DB
/** Verdadero si dos cadenas son iguales.*/
#define EQUALS(s1, s2) (strcmp(s1, s2) == 0)

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
    FILE_ADDED              /*!< Archivo adicionado  */
} return_code;

/**
 * @brief Inicializa el gestor de versiones 
 * @return int 0 si se inicializa correctamente, -1 si ocurre algún error
 */
int init_versions();

/**
 * @brief Crea una version en memoria del archivo
 * Valida si el archivo especificado existe y crea su hash
 * @param filename Nombre del archivo
 * @param hash Hash del contenido del archivo
 * @param comment Comentario
 * @param result Nueva version en memoria
 *
 * @return Resultado de la operacion
 */
return_code create_version(char *filename, char *comment, file_version *result);

/**
 * @brief Adiciona una nueva version de un archivo.
 *
 * @param v version de archivo a añadir
 * @param versions_db_path ruta completa al archivo de versiones
 *
 * @return 1 en caso de exito, 0 en caso de error.
 */
return_code add_new_version(file_version *v, char* versions_db_path);

/**
 * @brief Obtiene el hash de un archivo.
 * @param filename Nombre del archivo a obtener el hash
 * @param hash Buffer para almacenar el hash (HASH_SIZE)
 * @return Referencia al buffer, NULL si ocurre error
 */
char *get_file_hash(char *filename, char *hash);

/**
 * @brief imprime la estructura que guarda la version
 * @param v version de archivo que se imprimirá
 */
void print_version(const file_version *v);

/**
 * @brief Verifica si existe una version para un archivo
 *
 * @param filename Nombre del archivo
 * @param hash Hash del contenido
 *
 * @return 1 si la version existe, 0 en caso contrario.
 */
int version_exists(char *filename, char *hash);

/**
 * @brief retorna la version del archivo indicada
 * @param filename Nombre del archivo del cual se busca la version
 * @param version Version del archivo
 * @param v en la cual se va a guardar el resultado
 * @param versions_db_path ruta completa al archivo de versiones
 * @return estructura con los datos del archivo
 */
int get_version(file_version *v, char *filename, int version, char* versions_db_path);

#endif
