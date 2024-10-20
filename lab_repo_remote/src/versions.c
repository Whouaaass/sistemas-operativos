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
 * @brief Copia un archivo
 *
 * @param source Archivo fuente
 * @param destination Destino
 *
 * @return -1 para errores en la funcion y 0 como estado correcto
 */
return_code copy(char *source, char *destination);
/**
 * @brief Almacena un archivo en el repositorio
 *
 * @param filename Nombre del archivo
 * @param hash Hash del archivo: nombre del archivo en el repositorio
 *
 * @return
 */
return_code store_file(char *filename, char *hash);

/**
 * @brief Almacena un archivo en el repositorio
 *
 * @param hash Hash del archivo: nombre del archivo en el repositorio
 * @param filename Nombre del archivo
 *
 * @return
 */
int retrieve_file(char *hash, char *filename);

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

return_code add(char *filename, char *comment) {
    file_version v;

    // 1. Crea la nueva version en memoria
    // Si la operacion falla, retorna VERSION_ERROR
    // create_version(filename, comment, &v)
    if (create_version(filename, comment, &v) == VERSION_ERROR)
        return VERSION_ERROR;

    // 2. Verifica si ya existe una version con el mismo hash
    // Retorna VERSION_ALREADY_EXISTS si ya existe
    if (version_exists(filename, v.hash) == VERSION_ALREADY_EXISTS)
        return VERSION_ALREADY_EXISTS;

    // 3. Almacena el archivo en el repositorio.
    // El nombre del archivo dentro del repositorio es su hash (sin extension)
    // Retorna VERSION_ERROR si la operacion falla
    if (store_file(filename, v.hash) == VERSION_ERROR) return VERSION_ERROR;

    // 4. Agrega un nuevo registro al archivo versions.db
    // Si no puede adicionar el registro, se debe borrar el archivo almacenado
    // en el paso anterior
    if (add_new_version(&v) == VERSION_ERROR) {
        char dst_filename[PATH_MAX];
        snprintf(dst_filename, PATH_MAX, "%s/%s", VERSIONS_DIR, v.hash);
        if (remove(filename)) {
            perror("Error deleting file");
        } else {
            printf("File %s successfully deleted.\n", filename);
        }
        remove(dst_filename);
        return VERSION_ERROR;
    }
    // Si la operacion falla, retorna VERSION_ERROR

    // Si la operacion es exitosa, retorna VERSION_ADDED

    return VERSION_ADDED;
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

void list(char *filename) {
    FILE *fd;
    file_version v;
    ssize_t nread;
    int counter = 0;

    // Abre el la base de datos de versiones (versions.db)
    if ((fd = fopen(VERSIONS_DB_PATH, "rb")) == NULL) return;

    // Muestra los registros cuyo nombre coincide con filename.
    // Si filename es NULL, muestra todos los registros.
    while (nread = fread(&v, sizeof v, 1, fd), nread == 1) {
        if (filename == NULL) {
            print_version(&v);
        } else if (EQUALS(filename, v.filename)) {
            printf("%i ", ++counter);
            print_version(&v);
        }
    }

    fclose(fd);
}

char *get_file_hash(char *filename, char *hash) {
    struct stat s;

    // Verificar que el archivo existe y que se puede obtener el hash
    if (stat(filename, &s) < 0 || !S_ISREG(s.st_mode)) {
        perror("stat");
        return NULL;
    }

    sha256_hash_file_hex(filename, hash);

    return hash;
}

return_code copy(char *source, char *destination) {
    // Copia el contenido de source a destination (se debe usar
    // open-read-write-close, o fopen-fread-fwrite-fclose)
    // int fd_source, fd_destination;
    FILE *fd_source, *fd_destination;

    char buf[READ_CHUNK + 1];
    ssize_t nread;
    int saved_errno;

    if ((fd_source = fopen(source, "r")) == NULL) return VERSION_ERROR;

    if ((fd_destination = fopen(destination, "w")) == NULL) {
        fclose(fd_source);
        return VERSION_ERROR;
    }

    while (nread = fread(buf, sizeof(char), READ_CHUNK, fd_source), nread > 0) {
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = fwrite(out_ptr, sizeof(char), nread, fd_destination);
            if (nwritten >= 0) {
                nread -= nwritten;
                out_ptr += nwritten;
            }
        } while (nread > 0);
    }

    if (nread == 0) {
        fclose(fd_source);
        fclose(fd_destination);
        /* Success! */
        return FILE_ADDED;
    }

    return VERSION_ERROR;
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

return_code get(char *filename, int version) {
    file_version r;

    // 1. Abre la BD y busca el registro r que coincide con filename y version
    if (get_version(&r, filename, version) == VERSION_NOT_FOUND)
        return VERSION_NOT_FOUND;

    // 2. recupera el archivo
    return retrieve_file(r.hash, r.filename);
}

return_code store_file(char *filename, char *hash) {
    char dst_filename[PATH_MAX];
    snprintf(dst_filename, PATH_MAX, "%s/%s", VERSIONS_DIR, hash);
    return copy(filename, dst_filename);
}

int retrieve_file(char *hash, char *filename) {
    char src_filename[PATH_MAX];
    snprintf(src_filename, PATH_MAX, "%s/%s", VERSIONS_DIR, hash);
    return copy(src_filename, filename);
}

void print_version(const file_version *v) {
    int hash_length = strlen(v->hash);
    printf("%s %.3s...%.3s %s\n", v->filename, v->hash,
           (v->hash + hash_length - 3), v->comment);
}
