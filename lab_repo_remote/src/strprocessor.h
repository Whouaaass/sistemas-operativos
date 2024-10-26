/**
 * @file strprocessor.h
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @author Jorge Andrés Martinez Varón <jorgeandre@unicauca.edu.co>
 * @brief Manejo personalizado de strings 
 * 
 * @copyright MIT License
 * 
 */
#ifndef STRPROCESSOR_H
#define STRPROCESSOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "versions.h"

/**
 * @brief esta función procesa un buffer que va a ser tratado como una línea de comandos
 * @param cmdline línea de comandos
 * @param argc número de argumentos (se guarda en *argc) 
 * @author Joakim Söderberg <https://github.com/JoakimSoderberg> 
 * @return char** vector de argumentos
 * @note se debe de llamar a free para liberar la memoria asignada a los argumentos
 */
char **split_commandline(const char *cmdline, int *argc);

#endif