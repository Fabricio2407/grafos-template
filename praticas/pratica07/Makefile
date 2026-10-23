CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -std=c11
EXE =
ifeq ($(OS),Windows_NT)
EXE = .exe
endif
TARGET = main$(EXE)
TEST = tests/test_bellman_ford$(EXE)
SRC = bellman_ford.c main.c

all: $(TARGET)

$(TARGET): $(SRC) bellman_ford.h
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

check: $(TEST)
	./$(TEST)

$(TEST): tests/test_bellman_ford.c bellman_ford.c bellman_ford.h
	$(CC) $(CFLAGS) -I. bellman_ford.c tests/test_bellman_ford.c -o $(TEST)

clean:
	rm -f main main.exe tests/test_bellman_ford tests/test_bellman_ford.exe

.PHONY: all run check clean
