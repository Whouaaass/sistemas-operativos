Elaborado por: 
Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>

# Enunciado del problema

Se deberá desarrollar dos programas en C que implementen la funcionalidad de conectar un servidor con uno, y posiblemente varios clientes, a través de sockets IPv4. 

El programa servidor recibirá conexiones en un puerto especificado por argumentos de línea de comandos. 

El programa servidor recibirá por línea de comandos: la dirección IPv4 del servidor y el puerto a conectarse. Una vez que el cliente y el servidor se encuentren conectados, primero el cliente enviará un mensaje al servidor, quien deberá recibirlo y enviarlo de
vuelta. Posteriormente, el servidor y el cliente intercambiarán mensajes hasta que el usuario decida terminar el programa. 

Tanto el servidor como el cliente deben responder adecuadamente a las señales SIGINT y SIGTERM.

# Documentacion del Makefile
Genera los ejecutables
``` shell
make
```
Limpia los archivos para subir el proyecto
``` shell
make clean
```
Genera la documentacion 
``` shell
make doc
```
