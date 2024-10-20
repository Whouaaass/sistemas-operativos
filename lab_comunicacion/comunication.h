#ifndef COMUNICATION_H
#define COMUNICATION_H
/**
 * @file
 * @brief Comunication process
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @copyright MIT License
 */
#include <sys/types.h>

/**
 * @brief keeps a continous comunication between a server and a client
 * @param s aimed socket
 */
void keep_comunicating(int s, char *username);

#endif
