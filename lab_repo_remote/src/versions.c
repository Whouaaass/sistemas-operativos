/**
 * @file
 * @brief Implementacion del API de gestion de versiones
 * @author Erwin Meza Vega <emezav@unicauca.edu.co>
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andres Martinez Varon <jorgeandre@unicauca.edu.co>
 * @copyright MIT License
 */

#include "versions.h"

#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>

/**
 * @brief Imprime la estructura que guarda la version
 * @param v version de archivo que se imprimirá
 */
void print_version(const file_version *v);

int init_versions() {
    struct stat vfile_stat; /* Estado del archivo versions.db */

    // Crear el directorio ".versions/" si no existe
#ifdef __linux__
    mkdir(VERSIONS_DIR, 0755);
#elif _WIN32
    mkdir(VERSIONS_DIR);
#endif

    // Crea el archivo .versions/versions.db si no existe
    if (stat(VERSIONS_DB_PATH, &vfile_stat) != 0) {
        creat(VERSIONS_DB_PATH, 0755);
    }
    return 0;
}

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

return_code add_new_version(file_version *v, char* versions_db_path) {
    FILE *fp;
    fp = fopen(versions_db_path, "ab");
    if (fp == NULL) {
        return VERSION_ERROR;
    }
    flockfile(fp);

    // Adiciona un nuevo registro (estructura) al archivo versions.db
    fwrite(v, sizeof *v, 1, fp);

    funlockfile(fp);
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

int version_exists(char *filename, char *hash, char* versions_db_path) {
    FILE *fp;
    ssize_t nread;
    file_version v;

    // abrimos el archivo de versiones para leer el contenido
    if ((fp = fopen(versions_db_path, "r")) == NULL) return -1;

    flockfile(fp);
    while (nread = fread(&v, sizeof v, 1, fp), nread > 0) {
        if (EQUALS(filename, v.filename) && EQUALS(hash, v.hash)) {
            funlockfile(fp);
            fclose(fp);
            return VERSION_ALREADY_EXISTS;
        }
    }

    funlockfile(fp);
    fclose(fp);
    // Verifica si en la bd existe un registro que coincide con filename y hash
    return 1;
}

int get_version(file_version *v, char *filename, int version, char* versions_db_path) {
    FILE *fp;
    ssize_t nread;
    int count = 0;
    if ((fp = fopen(versions_db_path, "r")) == NULL) return -1;
    flockfile(fp);
    while (nread = fread(v, sizeof *v, 1, fp), nread > 0) {
        if (EQUALS(filename, v->filename) && ++count == version) {
            fclose(fp);
            return VERSION_OK;
        }
    }
    funlockfile(fp);
    fclose(fp);
    return VERSION_NOT_FOUND;
}

void print_version(const file_version *v) {
    int hash_length = strlen(v->hash);
    printf("%s %.3s...%.3s %s\n", v->filename, v->hash,
           (v->hash + hash_length - 3), v->comment);
}
