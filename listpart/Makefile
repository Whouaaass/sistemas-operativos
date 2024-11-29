# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lm

# Targets and dependencies
TARGET = listpart
OBJECTS = main.o mbr.o gpt.o utils.o

# Default target
all: $(TARGET)

# Linking rule
$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compilation rule (implicit)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

doc:
	doxygen

clean:
	rm -rf *.o $(TARGET) docs

install: all
	sudo cp listpart /usr/local/bin

uninstall:
	sudo rm -f /usr/local/bin/listpart