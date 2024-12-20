/**
 * @file
 * @brief Sistema de Control de Versiones
 * @author Erwin Meza Vega <emezav@gmail.com>
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andres Martinez Varon <jorgeandre@unicauca.edu.co>
 * Sistema de Control de Versiones
 * Uso: 
 *      versions add ARCHIVO "Comentario" : Adiciona una version del archivo al repositorio
 *      versions list ARCHIVO             : Lista las versiones del archivo existentes
 *      versions list                     : Lista todos los archivos almacenados en el repositorio
 *      versions get NUMBER ARCHIVO       : Obtiene una version del archivo del repositorio
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

#include "versions.h"

/**
 * @brief Imprime la ayuda
 */
void usage();

/**
 * @brief verifica si la direccion del archivo
 * está en el directorio actual
 * @param filepath Direccion del archivo
 * @return 1 si esta en el directorio, 0 si está en otro directorio
 */
int isoncurrentdir(char* filepath);

int main(int argc, char *argv[]) {
	struct stat s;
	int r_code;


	//Crear el directorio ".versions/" si no existe
#ifdef __linux__
	mkdir(VERSIONS_DIR, 0755);
#elif _WIN32
	mkdir(VERSIONS_DIR);
#endif

	// Crea el archivo .versions/versions.db si no existe
	if (stat(VERSIONS_DB_PATH, &s) != 0) {
		creat(VERSIONS_DB_PATH, 0755);
	}

	// Validar argumentos de linea de comandos
	if (argc == 4
			&& EQUALS(argv[1], "add")) {
		char *filename = basename(argv[2]);
		// Verifica si se trata de aniadir un archivo de otro directorio
		if (!isoncurrentdir(argv[2])) {
			fprintf(stderr, "Solo es posible adicionar archivos de este directorio\n");
			exit(EXIT_FAILURE);
		}

		// Aniade archivos del repositorio actual
		r_code = add(filename, argv[3]);
		if (r_code == VERSION_ERROR) {
		    fprintf(stderr, "No se puede adicionar %s\n", filename);
		} else if (r_code == VERSION_ALREADY_EXISTS) {
		    fprintf(stdout, "La version ya existe\n");
		}
	}else if (argc == 2
			&& EQUALS(argv[1], "list")) {
		//Listar todos los archivos almacenados en el repositorio
		list(NULL);
	}else if (argc == 3
			&& EQUALS(argv[1], "list")) {
		//Listar el archivo solicitado
		char *filename = basename(argv[2]);
		list(filename);
	}else if (argc == 4
			&& EQUALS(argv[1], "get")) {
		int version = atoi(argv[2]);
		char *filename = basename(argv[3]);
		if (version <= 0) {
		    fprintf(stderr, "Numero de version invalido\n");
			exit(EXIT_FAILURE);
		}
		//Obtiene la version especificada de un archivo
		r_code = get(filename, version);
		if (r_code == VERSION_ERROR) {
		    fprintf(stderr, "No se puede obtener la version %d de %s\n", version, filename);
			exit(EXIT_FAILURE);
		} else if (r_code == VERSION_NOT_FOUND) {
		    fprintf(stderr, "No se encontro la version %d de %s\n", version, filename);
			exit(EXIT_FAILURE);
		}
	}else {
		usage();
	}

	exit(EXIT_SUCCESS);

}

int isoncurrentdir(char* filepath) {
	char* dir = dirname(filepath);
	char actualpath[PATH_MAX], inputpath[PATH_MAX];
	if (EQUALS(realpath(".", actualpath), realpath(dir, inputpath))) {
		return 1;
	}
	return 0;
}

void usage() {
	printf("Uso: \n");
	printf("versions add ARCHIVO \"Comentario\" : Adiciona una version del archivo al repositorio\n");
	printf("versions list ARCHIVO             : Lista las versiones del archivo existentes\n");
	printf("versions list                     : Lista todos los archivos almacenados en el repositorio\n");
	printf("versions get NUMBER ARCHIVO       : Obtiene una version del archivo del repositorio\n");
}
