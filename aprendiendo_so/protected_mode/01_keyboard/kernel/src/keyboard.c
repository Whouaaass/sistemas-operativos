/**
 * @file keyboard.h
 * @brief Interfaz del manejador del teclado
 * @author Fredy Anaya <fredyanaya@unicauca.edu.co>
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright MIT License
 */

#include <keyboard.h>
#include <asm.h>
#include <irq.h>
#include <console.h>

/** @brief Tamano del mapa de teclado */
#define KEYMAP_SIZE 72

/** @brief bit que define si el codigo de la tecla corresponde a un liberamiento de tecla */
#define IS_KEY_BREAK 0x80

/** @brief bit del status que define si la tecla esta presionada, para teclas especiales*/
#define IS_SPECIAL_KEY_PRESSED 0b00000001
/** @brief bit del status que define si la tecla esta activa, solo aplica para teclas del tipo "bloq"/"lock" */ 
#define IS_SPECIAL_KEY_ACTIVE 0b00000010

/** @brief Estado de la tecla shift ()*/
unsigned char shift_status = 0;
/** @brief Estado de la tecla ctrl (1 para presionada, 0 no presionada)*/
unsigned char ctrl_status = 0;
/** @brief Estado de la tecla Bloq Mayus (2 para activa, 1 para inactiva)*/
unsigned char capslock_status = 0;

/** @brief este es el mapa de las teclas */
unsigned char keyboard_map[KEYMAP_SIZE] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
    '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
    '\n', 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+'
};

/** @brief este es el mapa de las teclas con shift presionado */
unsigned char keyboard_shift_map[KEYMAP_SIZE] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
    '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',
    '\n', 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 
    0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+'
};

/**
 * @brief Rutina de atención de interrupción del teclado
 */
void keyboard_handler();

/**
 * @brief Maneja las teclas especiales
 * 
 * @param scancode codigo de la tecla por el teclado
 */
void handle_special_keys(unsigned char scancode);

/**
 * @brief traduce el codigo de la tecla a un caracter ASCII
 * 
 * @param scancode codigo de la tecla por el teclado
 * @return char caracter ASCII
 */
char to_ascii_key(unsigned char scancode);

void setup_keyboard() {
    // Habilitar interrupciones del teclado        
    console_printf("Setting up keyboard...\n");
    // limpia las teclas presionadas        
    install_irq_handler(IRQ_KEYBOARD, keyboard_handler);
}

void keyboard_handler() {

    unsigned char status = inb(KEYBOARD_STATUS_PORT);

    if (!(status & IS_KEYBOARD_BUSY)) return;

    // Leer el codigo escaneado del teclado
    unsigned char scancode = inb(KEYBOARD_DATA_PORT);

    // Se manejan las teclas especiales
    handle_special_keys(scancode);

    // determinar si la tecla se esta presionando o liberando    

    unsigned char ascii_key = to_ascii_key(scancode);    

    // Tecla presionada
    //console_printf("Tecla presionada: %c\n", keyboard_map[scancode]);
    //console_printf("%c hex: %x dec: %d is_break: %d\n", scancode < KEYMAP_SIZE ? keyboard_map[scancode] : 0, scancode, scancode, is_break);
    if (ascii_key) console_putchar(ascii_key);
}

void handle_special_keys(unsigned char scancode) {
    switch (scancode) {
        case 0x2A: // Shift presionado
            shift_status = 1;
            break;
        case 0xAA: // Shift liberado
            shift_status = 0;
            break;
        case 0x1D: // Ctrl presionado
            ctrl_status = 1;
            break;
        case 0x9D: // Ctrl liberado
            ctrl_status = 0;
            break;
        case 0x3A: // Bloq Mayus presionado
            if (capslock_status & IS_SPECIAL_KEY_PRESSED) break;
            // voltea el bit de IS_SPECIAL_KEY_ACTIVE y activa el bit de IS_SPECIAL_KEY_PRESSED
            capslock_status = capslock_status ^ IS_SPECIAL_KEY_ACTIVE | IS_SPECIAL_KEY_PRESSED;            
            break;
        case 0xBA: // Bloq Mayus liberado
            // desactiva el bit de IS_SPECIAL_KEY_PRESSED
            capslock_status = capslock_status & ~IS_SPECIAL_KEY_PRESSED;            
            break;
    }
}

char to_ascii_key(unsigned char scancode) {
    if (scancode & IS_KEY_BREAK || scancode > KEYMAP_SIZE) return 0;
    unsigned char ascii_key = keyboard_map[scancode];
    unsigned char is_caps_applicable = (capslock_status & IS_SPECIAL_KEY_ACTIVE) && (ascii_key >= 'a' && ascii_key <= 'z');

    if (shift_status != is_caps_applicable) {
        return keyboard_shift_map[scancode];
    }
    return ascii_key;
}
