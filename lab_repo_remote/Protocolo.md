# Protocolo de comunicación
Esta es una descripción de los pasos del protocolo de comunicación que tiene el cliente con el servidor para hacer cada petición

## Para cada petición
Primero se debe de hacer el saludo, para confirmar la correcta conexión con el servidor,
en este saludo se pretende confirmar que el servidor esta usando el protocolo adecuado,
por lo que para una sencilla implementación va a ser un código de palabras que las dos partes,
va a tener que reconocer:
	- Servidor: "Versiones"
	- Cliente: "Remotas"

## Add
El método add comprende la adición de archivos al servidor
### Cliente
1. Manda el nombre/identificador del método ("ADD")
2. Manda hash del archivo, con su nombre y comentario
3. Recibe una respuesta indicando si es necesario subir el archivo (está actualizado)
4. Si no es necesario subir el archivo, termina la conexión
5. Manda el tamaño del archivo
6. Manda el archivo
7. Recibe una respuesta que indica si el archivo se subio correctamente
8. Cierra la conexión con el servidor
### Servidor
1. Recibe el nombre/identificador ("ADD") y se prepara para el otro mensaje
2. Recibe el hash del archivo, con su nombre
3. Manda una respuesta indicando si el archivo está actualizado o no
4. Si el archivo está actualizado, termina la conexión
5. Recibe el tamaño del archivo
6. Recibe el archivo
7. Manda información acerca de si el archivo se subio con exito, (Se compara con el hash, mandado al principio)
8. Cierra la conexión

## Get
El método get comprende el envio de un archivo al cliente
### Cliente
1. Manda el nombre/identificador del método ("GET")
2. Manda el nombre del archivo, junto con la versión que desea obtener
3. Recibe el hash del archivo del servidor o se indica que no existe tal archivo o versión
4. Si el archivo no existe termina la conexión
5. Manda al servidor una indicación si necesitá descargar el archivo de este (Está desactualizado)
6. Si no es necesario subir el archivo, termina la conexión
7. Recibe el tamaño del archivo
8. Recibe el archivo	
9. Cierra conexión
### Servidor
1. Recibe el nombre/identificador del método ("GET")
2. Recibe el nombre del archivo, junto con su versión
3. Si el archivo no existe, manda un mensaje diciendo que la versión o el archivo no existe y termina la conexión
4. Manda el hash de la ultima versión del archivo en el servidor
5. Recibe o no un mensaje del cliente indicando si necesita descargar el archivo
6. Si no se recibe un mensaje o si el mensaje es que no necesita el archivo termina conexión
7. Manda el tamaño del archivo
8. Manda el archivo
9. Cierra conexión

## List
El método consiste en listar las versiones de un archivo ("nombre del archivo", "numero version", "hash"), el numero de versión se infiere
Se puede especificar el archivo del cual se desee listar las versiones, de lo contrario se deberán mostrar las versiones de todos los archivos
### Cliente
1. Manda el nombre/identificador del método ("LIST")
2. Manda el nombre del archivo o manda 0's
3. Recibe el tamaño de la lista a recibir
4. Recibe la lista
5. Cierra conexión
### Servidor
1. Recibe el nombre/identificador del método ("LIST")
2. Recibe el nombre del archivo o manda 0's
3. Manda el tamaño de la lista
4. Manda la lista
5. Cierra la conexión

	`Fredy Esteban Anaya Salazar`
