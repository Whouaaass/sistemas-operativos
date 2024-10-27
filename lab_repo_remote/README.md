**Elaborado por**:

- Erwin Meza Vega <emezav@unicauca.edu.co> `profesor`
- Fredy Esteban Anaya Salazar <fredyanaya@unicauca.edu.co>
- Jorge Andres Martinez Varon <jorgeandre@unicauca.edu.co>

# Sistema de Control de Versiones Remoto
Los Sistemas de Control de Versiones (VCS) permiten guardar el rastro de las modificaciones sobre determinados elementos. En el contexto de este proyecto, se gestionarán versiones de archivos y directorios.

Se deberá implementar un sistema de control de versiones remoto, que permita:
* Adicionar un archivo al repositorio de versiones.
* Listar las versiones de un archivo en el repositorio de versiones.
* Listar todos los archivos almacenados en el repositorio.
* Obtener la versión de un archivo del repositorio de versiones.

Se deberán crear dos (programas), un programa servidor llamado `rversionsd`,
y un programa cliente llamado `rversions`.

## Uso del cliente rversions

```shell
$ ./rversions
Uso: rversions IP PORT Conecta el cliente a un servidor en la IP y puerto especificados.

Los comandos, una vez que el cliente se ha conectado al servidor, son los siguientes:
	add ARCHIVO "COMENTARIO"
	list ARCHIVO
	list
	get NUMVER ARCHIVO
```

## Uso del servidor rversionsd
```shell
$ ./rversionsd 
Uso: rversionsd PORT Escucha por conexiones del cliente en el puerto especificado. 
	./rversionsd PORT
```

## Repositorio de versiones

El repositorio de versiones funcionará como un servidor que mediante sockets.
permitirá la conexión de uno o más clientes. Una vez iniciado, deberá crear un directorio llamado "files", el el cual se almacenarán todos los archivos del cliente. 
Para esta versión el manejo de directorios y subdirectorios es opcional. El sevidor puede almacenar todos los archivos del cliente en el directorio files sin crear subdirectorios.  
Dado que este programa solo será usado por un usuario (cliente), para esta versión no se requiere implementar mecanismos de autenticación.
Opcionalmente, Para una calificación de 5.0 en el primer y segundo corte,
sin importar la calificación obtenida en el primer corte, se puede implementar la lógica para almacenar archivos de múltiples usuarios.  
Se deberá diseñar e implementar el PROTOCOLO (estructura y secuencia de los mensajes) que permiten enviar y recibir los archivos entre el cliente y el servidor.

## Detección de cambios

El esquema para detección de cambios en un archivo puede ser muy com plejo. Para la implementación de este programa, se usará el módulo auxiliar sha256, el cual implementa el algoritmo del mismo nombre y permite obtener el código SHA de 256 bits (64 bytes + NULL). (ver función get_file_hash en el código base proporcionado).