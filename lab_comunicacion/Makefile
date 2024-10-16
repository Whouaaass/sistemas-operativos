all: client.o server.o protocol.o
	gcc -o -g server server.o protocol.o
	gcc -o -g client client.o protocol.o

## Compila todos los archivos *.o
%.o: %.c
	gcc -c -g $< -o $@


clean:
	rm -rf *.o client server docs

doc:
	doxygen
