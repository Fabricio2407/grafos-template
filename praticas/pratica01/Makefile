CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -lm
TARGET = pratica01
SRC = *.c
OBJ = *.o

all: main

main: $(SRC)
	$(CC) $(CFLAGS) -o main $(SRC)

run: main
	./main

clean:
	rm -f main main.exe $(TARGET) $(TARGET).exe $(OBJ)

.PHONY: all clean run
