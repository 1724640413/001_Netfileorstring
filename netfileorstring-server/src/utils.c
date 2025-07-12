#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to read a file and return its contents
char* read_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(filesize + 1);
    if (!buffer) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, filesize, file);
    buffer[filesize] = '\0'; // Null-terminate the string
    fclose(file);
    return buffer;
}

// Function to write data to a file
int write_file(const char* filename, const char* data) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file");
        return -1;
    }

    fwrite(data, sizeof(char), strlen(data), file);
    fclose(file);
    return 0;
}

// Function to handle a client connection
int handle_client( int client_socket )
{
    char buffer[1024];
    int bytes_read;

    // 读取客户端送达数据
    bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_read == -1) {
        perror("Failed to read from socket");
        return -1;
    }

    // 处理客户端送达的数据
    printf("Received data: %s\n", buffer);
    

    return 0;
}