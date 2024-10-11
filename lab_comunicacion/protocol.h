/**
 * @file
 * @brief Interfaz Protocolo de comunicación
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @copyright MIT License
 */

/**
 * @brief Envía un mensaje de saludo
 * @param s Socket al que se envia el mensaje
 */
int send_greeting(int s);

/**
 * @brief Recibe un mensaje del saludo
 * @param s Socket del que se rcibe el mensaje
 */
int receive_greeting(int s);
