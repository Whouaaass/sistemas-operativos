# Constants
P_NAME=vcs_fredyanaya_jorgeandre


# Commands
all: main.o sha256.o versions.o
	gcc -o versions main.o sha256.o versions.o

main.o: main.c
	gcc -g -c -o main.o main.c

sha256.o: sha256.c
	gcc -g -c -o sha256.o sha256.c

versions.o: versions.c
	gcc -g -c -o versions.o versions.c

clean:
	rm -f versions *.o *.zip
	rm -rf docs

clean-repo:
	rm -rf .versions

doc:
	doxygen

install: all
	echo "instalando versions"
	sudo cp versions /usr/local/bin

uninstall:
	sudo rm -f /usr/local/bin/versions

zip: clean clean-repo
	rm -rf *.zip ${P_NAME}
	mkdir ${P_NAME}
	rsync -av --exclude=${P_NAME} * ${P_NAME}
	zip -r9 ${P_NAME}.zip ${P_NAME}
	rm -r ${P_NAME}
