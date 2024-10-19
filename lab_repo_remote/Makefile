
# Compiler
CC = gcc

# Directories
SRC_DIR = src
OUT_DIR = out

# Find all .c files in the src directory
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)

# Generate corresponding .o files in the out directory
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OUT_DIR)/%.o, $(SRC_FILES))

# Target to compile all .o files
all: $(OBJ_FILES)
	$(CC) -o rversions $(OUT_DIR)/rversions.o $(OUT_DIR)/sha256.o
	$(CC) -o rversionsd $(OUT_DIR)/rversionsd.o $(OUT_DIR)/sha256.o

# Rule to compile .c files to .o files
$(OUT_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OUT_DIR)      # Create output directory if it doesn't exist
	$(CC) -c $< -o $@

# Clean up
clean:
	rm -f -r $(OUT_DIR)/*.o rversions rversionsd docs

# Make documentation
doc:
	doxygen

.PHONY: all clean
