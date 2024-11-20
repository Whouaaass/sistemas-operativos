/**
 * @file userauth.c
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @brief Implementación de la autenticación de usuarios
 *
 * @copyright MIT License
 */

#include "userauth.h"

#include <stdio.h>
#include <string.h>

/* Mutex para la base de datos de los clientes */
pthread_mutex_t userdb_lock;

int init_userauth() {
    struct stat vfile_stat; /* Estado del archivo versions.db */

    // crea el directorio principal si no existe
#ifdef __linux__
    mkdir(USERS_DIR, 0755);
#elif _WIN32
    mkdir(USERS_DIR);
#endif

    // Crea el archivo users.db si no existe
    if (stat(USERS_DB_PATH, &vfile_stat) != 0) {
        creat(USERS_DB_PATH, 0755);
    }

    if (pthread_mutex_init(&userdb_lock, NULL)) {
        perror("Error initializing user database mutex");
        exit(EXIT_FAILURE);
    }

    return 0;
}

void save_user(char *username, char *password, int s) {
    FILE *fp;
    user_record user;
    
    memset(&user, 0, sizeof(user_record));
    snprintf(user.username, USERNAME_SIZE, "%s", username);
    snprintf(user.password, PASSWORD_SIZE, "%s", password);  
    
    pthread_mutex_lock(&userdb_lock);
    if ((fp = fopen(USERS_DB_PATH, "ab")) == NULL) {
        perror("Error Abriendo el archivo");
        return;
    }
    fwrite(&user, sizeof(user_record), 1, fp);
    fclose(fp);
    pthread_mutex_unlock(&userdb_lock);
}

int search_user(char *username, user_record *user) {     
    memset(user, 0, sizeof(user_record));
    pthread_mutex_lock(&userdb_lock);
    FILE *fp;
    if ((fp = fopen(USERS_DB_PATH, "r")) == NULL) {
        perror("Error Abriendo el archivo");
        return -1;
    }
    while (fread(user, sizeof(user_record), 1, fp) == 1) {
        if (EQUALS(user->username, username)) {
            fclose(fp);
            pthread_mutex_unlock(&userdb_lock);
            return 0;
        }
    }
    fclose(fp);
    pthread_mutex_unlock(&userdb_lock);
    return -1;
}