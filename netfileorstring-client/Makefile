CC = gcc
CFLAGS = -Wall -g -fPIC
SRCS = src/main.c src/client.c 
OBJS = $(SRCS:.c=.o)
TARGET = client
LIBDIR = dell
LIBNAME = libclient.so

all: $(TARGET) $(LIBDIR)/$(LIBNAME)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lssl -lcrypto

$(LIBDIR)/$(LIBNAME): src/client.o
	mkdir -p $(LIBDIR)
	$(CC) -shared -o $@ src/client.o -lssl -lcrypto

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)