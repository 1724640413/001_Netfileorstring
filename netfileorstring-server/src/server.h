#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


void start_server();
void save_file(const char *filename, const char *data, size_t data_length);
void handle_client_data(int client_socket);


#endif // SERVER_H