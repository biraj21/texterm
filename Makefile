CC = cc
FLAGS = -Wall -Werror -Wpedantic -Wextra
SRC = src
OBJ = obj
BIN = texterm
LANGUAGE_FILES = $(wildcard $(SRC)/languages/*.h)
SRC_FILES = $(wildcard $(SRC)/*.[ch])
OBJ_FILES = $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(wildcard $(SRC)/*.c))

all: setup clean $(BIN)

# set up the project by creating 'src' & 'obj' directories
setup:
	mkdir -p src obj

# clean up by deleting binary & object files
clean:
	rm -f $(BIN) $(OBJ)/*.o sample

# create binary by stitching object files
$(BIN): $(OBJ_FILES)
	$(CC) $(FLAGS) -o $@ $(OBJ_FILES)

# compile main.c when any source file has changed
$(OBJ)/main.o: $(SRC_FILES)
	$(CC) $(FLAGS) -c -o $@ $(SRC)/main.c

# compile highlight.c only when highlight.c or any language header files have changed
$(OBJ)/highlight.o: $(SRC)/highlight.c $(LANGUAGE_FILES)
	$(CC) $(FLAGS) -c -o $@ $(SRC)/highlight.c

# compile any other file only when it or any header files have changed
$(OBJ)/%.o: $(SRC)/%.c $(SRC)/%.h
	$(CC) $(FLAGS) -c -o $@ $<

debug: $(BIN)
	valgrind --log-file=valgrind.log $(BIN)
