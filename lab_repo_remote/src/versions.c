/**
 * @file
 * @brief Implementacion del API de gestion de versiones
 * @author Erwin Meza Vega <emezav@unicauca.edu.co>
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andres Martinez Varon <jorgeandre@unicauca.edu.co>
 * @copyright MIT License
 */

#include "versions.h"

#include <libgen.h>

/**
 * @brief Imprime la estructura que guarda la version
 * @param v version de archivo que se imprimirá
 */
void print_version(const file_version *v);

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
 * @return estructura con los datos del archivo
 */
int get_version(file_version *v, char *filename, int version);

/**
 * @brief Obtiene el hash de un archivo.
 * @param filename Nombre del archivo a obtener el hash
 * @param hash Buffer para almacenar el hash (HASH_SIZE)
 * @return Referencia al buffer, NULL si ocurre error
 */
char *get_file_hash(char *filename, char *hash);


/**
 * @brief Adiciona una nueva version de un archivo.
 *
 * @param filename Nombre del archivo.
 * @param comment Comentario de la version.
 * @param hash Hash del contenido.
 *
 * @return 1 en caso de exito, 0 en caso de error.
 */
return_code add_new_version(file_version *v);

return_code create_version(char *filename, char *comment,
                           file_version *result) {
    struct stat s;
    char hash[HASH_SIZE];

    // 1. Valida que el archivo exista y sea un archivo regular
    if (stat(filename, &s) < 0 || !S_ISREG(s.st_mode)) {
        return VERSION_ERROR;
    }

    // 2. Obtiene el HASH del archivo
    get_file_hash(filename, hash);

    // Se limpia la estructura
    memset(result, 0, sizeof *result);

    // Llena todos los atributos de la estructura y retorna VERSION_CREATED
    strcpy(result->filename, filename);
    strcpy(result->comment, comment);
    strcpy(result->hash, hash);

    // En caso de fallar alguna validacion, retorna VERSION_ERROR

    return VERSION_CREATED;
}

return_code add_new_version(file_version *v) {
    FILE *fp;
    fp = fopen(VERSIONS_DB_PATH, "ab");
    if (fp == NULL) {
        return VERSION_ERROR;
    }

    // Adiciona un nuevo registro (estructura) al archivo versions.db
    fwrite(v, sizeof *v, 1, fp);

    fclose(fp);
    return VERSION_CREATED;
}



char *get_file_hash(char *filename, char *restrict hash) {
    struct stat s;

    // Verificar que el archivo existe y que se puede obtener el hash
    if (stat(filename, &s) < 0 || !S_ISREG(s.st_mode)) {
        perror("stat");
        return NULL;
    }

    sha256_hash_file_hex(filename, hash);

    return hash;
}

int version_exists(char *filename, char *hash) {
    FILE *fp;
    ssize_t nread;
    file_version v;

    // abrimos el archivo de versiones para leer el contenido
    if ((fp = fopen(VERSIONS_DB_PATH, "r")) == NULL) return -1;

    while (nread = fread(&v, sizeof v, 1, fp), nread > 0) {
        if (EQUALS(filename, v.filename) && EQUALS(hash, v.hash)) {
            fclose(fp);
            return VERSION_ALREADY_EXISTS;
        }
    }

    fclose(fp);
    // Verifica si en la bd existe un registro que coincide con filename y hash
    return 1;
}

int get_version(file_version *v, char *filename, int version) {
    FILE *fp;
    ssize_t nread;
    int count = 0;
    if ((fp = fopen(VERSIONS_DB_PATH, "r")) == NULL) return -1;
    while (nread = fread(v, sizeof *v, 1, fp), nread > 0) {
        if (EQUALS(filename, v->filename) && ++count == version) {
            fclose(fp);
            return VERSION_OK;
        }
    }
    fclose(fp);
    return VERSION_NOT_FOUND;
}

void print_version(const file_version *v) {
    int hash_length = strlen(v->hash);
    printf("%s %.3s...%.3s %s\n", v->filename, v->hash,
           (v->hash + hash_length - 3), v->comment);
}
