CC = gcc
CFLAGS = -Wall -g
SRC = src/main.c src/server.c src/utils.c
OBJ = $(SRC:.c=.o)
TARGET = netfileorstring-server

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean