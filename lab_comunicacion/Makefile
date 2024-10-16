all: client.o server.o protocol.o comunication.o
	gcc -o server server.o protocol.o comunication.o
	gcc -o client client.o protocol.o comunication.o

## Compila todos los archivos a *.o
%.o: %.c
	gcc -c $< -o $@

clean:
	rm -rf *.o client server docs

doc:
	doxygen
