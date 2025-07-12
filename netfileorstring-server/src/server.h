#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 42323
#define BUFFER_SIZE 1024

void start_server();
void handle_client(int client_socket);
void save_file(const char *filename, const char *data, size_t data_length);
void handle_client_data(const char *buffer, int length);

#endif // SERVER_H