CC = gcc
CFLAGS = -O2 -Wall -Wextra -Wpedantic -Werror -std=c11
TARGET = main
ifeq ($(OS),Windows_NT)
TARGET = main.exe
endif
SRC = hamiltoniano.c tps.c main.c

all: $(TARGET)

$(TARGET): $(SRC) hamiltoniano.h tps.h
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

check: $(TARGET)
	./$(TARGET) --test

clean:
	rm -f main main.exe

.PHONY: all run check clean
