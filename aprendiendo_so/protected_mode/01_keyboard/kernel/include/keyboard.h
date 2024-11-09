/**
 * @file keyboard.h
 * @brief Interfaz del manejador del teclado
 * @author Fredy Anaya <fredyanaya@unicauca.edu.co> 
 * @copyright MIT License
 */
#ifndef KEYBOARD_H
#define KEYBOARD_H

/** @brief bit del status que define si el teclado esta ocupado */
#define IS_KEYBOARD_BUSY 0x01

/**
 * @brief Inicializa el manejador del teclado
 */
void setup_keyboard();


#endif