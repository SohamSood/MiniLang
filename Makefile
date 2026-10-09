CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -Iinclude
SRC = src/main.c src/lexer.c src/parser.c src/ast.c src/evaluator.c

ifeq ($(OS),Windows_NT)
    TARGET = minilang.exe
    RM = del /Q /F
else
    TARGET = minilang
    RM = rm -f
endif

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	$(RM) $(TARGET)

run: $(TARGET)
	./$(TARGET) demo.ml

test: $(TARGET)
	@echo Running MiniLang test suite...
	./$(TARGET) examples/hello.ml
	./$(TARGET) examples/02_precedence.ml
	./$(TARGET) examples/03_variables.ml
	./$(TARGET) examples/04_condition.ml
	./$(TARGET) examples/05_loop.ml
	./$(TARGET) examples/06_functions.ml
	./$(TARGET) examples/07_fibonacci.ml
