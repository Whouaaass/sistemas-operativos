/**
 * @file
 * @brief Programa Server
 * @author Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
 * @copyright MIT License
 */
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>


int main(int argc, char const *argv[])
{
    int s; // socket del servidor
    int c; // socket del cliente
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    // 0. instalar los manejadores de SIGINT, SIGTERM

    // 1. Obtener un conector
    s = socket(AF_INET, SOCK_STREAM, 0);

    // 2. Asociar una direccion a un conector - bind
    memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234); //TODO !!! -> debe de ser recibido por linea de comandos !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    addr.sin_addr.s_addr = INADDR_ANY; // -> 0.0.0.0

    if (bind(s, (struct sockaddr *) &addr, addr_len))
    {
    	printf("Algo malo sucedio con el bind\n");
     	exit(EXIT_FAILURE);
    }

    // 3. Colocar el socket disponible - listen
    if (listen(s, 1)) {
    	printf("Algo malo sucedio con el listen\n");
     	exit(EXIT_FAILURE);
    }

    // 4. (bloqueante) Esperar por un cliente `c` - accept
    c = accept(s, (struct sockaddr *) &addr, &addr_len);

    // 5. (comunicación)
    //      recibir el saludo
    //      enviar el saludo
    //      ....
    while (accept)
    {

    }

    // 6. cerrar el socket del cliente c
    // 7. cerrar el socket del servidor s
    exit(EXIT_FAILURE);
}
