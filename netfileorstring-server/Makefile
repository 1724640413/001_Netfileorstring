CC = gcc
CFLAGS = -Wall -g -fPIC
SRCS = src/main.c src/server.c src/utils.c
# SRCS = $(wildcard src/*.c) # 如果需要自动查找所有源
OBJS = $(SRCS:.c=.o)
TARGET = netfileorstring-server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lssl -lcrypto

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
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