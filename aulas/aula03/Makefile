CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11
LDFLAGS =
LDLIBS =
TARGET = main
SRC = main.c grafo_lista.c busca_grafo.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
