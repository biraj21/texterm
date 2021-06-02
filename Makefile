CC = gcc
FLAGS = -g -Wall -Werror
LANG_HEADERS = $(wildcard ./includes/languages/*.h)
SRC_FILES = $(wildcard ./src/*.c)
OBJ_FILES = $(patsubst ./src/%.c, ./obj/%.o, $(SRC_FILES))
BIN = texterm

$(BIN): obj $(OBJ_FILES)
	$(CC) $(FLAGS) $(OBJ_FILES) -o $@

obj:
	mkdir obj

obj/main.o: src/main.c
	$(CC) $(FLAGS) -c $< -o $@

obj/highlight.o: src/highlight.c includes/languages.h $(LANG_HEADERS)

obj/%.o: src/%.c includes/%.h 
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f ./obj/* $(BIN) test

test: test.c
	$(CC) $(FLAGS) $^ -o $@