#include <crypt.h>  // -lcrypt
#include <stdio.h>
#include <stdlib.h>

/* 
    Compilar con -lcrypt
    gcc -o cifrar cifrar.c -lcrypt
    
    
	Cifra un texto para shadow password usando algun metodo de cifrado definido en crypt(5)
   argc = 3
   argv[0] = ejecutable
   argv[1] = password en texto plano (entre comillas)
   argv[2] = prefijo del metodo de cifrado (sin $) ver crypt(5)
   */
int main(int argc, char * argv[]) {

	char prefix[32];

	if (argc != 3) {
		exit(EXIT_FAILURE);
	}

	//Crear el prefijo definido en crypt(5)
	sprintf(prefix, "$%s$", argv[2]);

	char * salt;

    //Generar la semilla
	salt = crypt_gensalt(prefix, 0, NULL, 0);

	if (salt == NULL) {
		exit(EXIT_FAILURE);
	}

	//crypt devuelve una cadena con el texto cifrado, enviar a la salida estandar.
	printf("%s\n", crypt(argv[1], salt));

	exit(EXIT_SUCCESS);
}
