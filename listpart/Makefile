all: main.o mbr.o gpt.o
	gcc -o listpart main.o mbr.o gpt.o -lm

main.o: main.c
	gcc -c -o main.o main.c

mbr.o: mbr.c
	gcc -c -o mbr.o mbr.c


gpt.o: gpt.c
	gcc -c -o gpt.o gpt.c


doc:
	doxygen

clean:
	rm -rf main.o listpart docs


install: all
	sudo cp listpart /usr/local/bin

uninstall:
	sudo rm -f /usr/local/bin/listpart