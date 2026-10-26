CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -std=c11
TARGET = main
ifeq ($(OS),Windows_NT)
TARGET = main.exe
endif
SRC = cobertura.c main.c

all: $(TARGET)

$(TARGET): $(SRC) cobertura.h
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

check: $(TARGET)
	./$(TARGET) --test

clean:
	rm -f main main.exe

.PHONY: all run check clean
