CC = gcc
FLAGS = -g -Wall -Werror
SRC_FILES = $(wildcard ./src/*.c)
OBJ_FILES = $(patsubst ./src/%.c, ./obj/%.o, $(SRC_FILES))
BIN = texterm

$(BIN): $(OBJ_FILES)
	$(CC) $(FLAGS) $^ -o $@

obj/main.o: src/main.c
	$(CC) $(FLAGS) -c $< -o $@

obj/highlight.o: src/highlight.c includes/languages.h

obj/%.o: src/%.c includes/%.h 
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f ./obj/* $(BIN) test

test: test.c
	$(CC) $(FLAGS) $^ -o $@