CC = gcc
CFLAGS = -Wall -g
SRC = src/main.c src/server.c src/utils.c
# SRCS = $(wildcard src/*.c) # 如果需要自动查找所有源
OBJ = $(SRC:.c=.o)
TARGET = netfileorstring-server

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
	rm -f dell/libserver.so
	rm -f /usr/local/include/netfileorstring-server.h
	rm -rf /usr/local/bin/netfileorstring-server

install:
	mkdir -p /usr/local/bin/netfileorstring-server
	cp $(TARGET) /usr/local/bin/netfileorstring-server/$(TARGET)
	chmod +x /usr/local/bin/netfileorstring-server/$(TARGET)
	cp src/server.h /usr/local/include/netfileorstring-server.h
	cp src/netfileorstring_monitor.sh /usr/local/bin/netfileorstring-server/netfileorstring_monitor.sh
	chmod +x /usr/local/bin/netfileorstring-server/netfileorstring_monitor.sh

.PHONY: all clean install