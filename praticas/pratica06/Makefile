CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -std=c11
TARGET = main
ifeq ($(OS),Windows_NT)
TARGET = main.exe
endif
SRC = dijkstra.c main.c

all: $(TARGET)

$(TARGET): $(SRC) dijkstra.h
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

check: tests/test_dijkstra
	./tests/test_dijkstra

tests/test_dijkstra: tests/test_dijkstra.c dijkstra.c dijkstra.h
	$(CC) $(CFLAGS) -I. dijkstra.c tests/test_dijkstra.c -o tests/test_dijkstra

clean:
	rm -f main main.exe tests/test_dijkstra tests/test_dijkstra.exe

.PHONY: all run check clean
