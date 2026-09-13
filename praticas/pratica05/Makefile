CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11
LDLIBS = -lm
TARGET = main
SRC = coloracao.c main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

clean:
	rm -f main main.exe

.PHONY: all clean
