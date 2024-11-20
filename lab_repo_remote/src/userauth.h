/**
 * @file userauth.h
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @brief Logica de autenticación de usuarios para el servidor
 * 
 * @copyright MIT License 
 */

#ifndef USERAUTH_H
#define USERAUTH_H

#include <pthread.h>

#include "versions.h"

#define USERNAME_SIZE 64
#define PASSWORD_SIZE 64

/* Directorio del repositorio */
#define USERS_DIR VERSIONS_DIR
/* Nombre de la base de datos de usuarios */
#define USERS_DB "users.db"
/* Ruta completa de la base de datos */
#define USERS_DB_PATH USERS_DIR "/" USERS_DB

typedef struct {
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];    
} user_record;

/**
 * @brief Inicializa la autenticación de usuarios 
 * @return 0 si se inicializa correctamente, -1 si ocurre algún error
 */
int init_userauth();

/**
 * @brief Guarda a un usuario
 * @param username nombre de usuario
 * @param password contraseña
 * @param s socket del cliente 
 * @note El nombre de usuario y la contraseña solo guardan 64 caracteres
 */
void save_user(char *username, char *password, int s);

/**
 * @brief busca el registro de un usuario 
 * @param username nombre del usuario   
 * @param user donde se guardará el registro del usuario
 * @return 0 si se encuentra el usuario, -1 si no se encuentra
 */
int search_user(char *username, user_record *user);

#endif