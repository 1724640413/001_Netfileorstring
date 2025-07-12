#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "server.h"

// 设置端口号
#define PORT 42323
// 缓存区大小
#define BUFFER_SIZE 1024

// 处理客户端数据
int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // 创建 socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 附加选项
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // 设置地址
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 绑定 socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 开始监听
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("服务器正在监听 %d 端口...\n", PORT);

    while (1) {
        // 接受客户端连接
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // 接收数据
        int bytes_read = read(new_socket, buffer, BUFFER_SIZE);
        if (bytes_read > 0) {
            // 处理接收到的数据
            handle_client_data(buffer, bytes_read);
        }

        close(new_socket);
    }

    return 0;
}



// int main() {
//     int server_socket, client_socket;
//     struct sockaddr_in server_addr, client_addr;
//     socklen_t client_addr_len = sizeof(client_addr);

//     // 创建套接字
//     server_socket = socket(AF_INET, SOCK_STREAM, 0);
//     if (server_socket < 0) {
//         perror("创建套接字失败");
//         exit(EXIT_FAILURE);
//     }

//     // 设置服务器地址结构
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_addr.s_addr = INADDR_ANY;
//     server_addr.sin_port = htons(PORT);

//     // 绑定套接字
//     if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
//         perror("绑定失败");
//         close(server_socket);
//         exit(EXIT_FAILURE);
//     }

//     // 开始监听
//     if (listen(server_socket, 5) < 0) {
//         perror("监听失败");
//         close(server_socket);
//         exit(EXIT_FAILURE);
//     }

//     printf("服务器正在运行，等待客户端连接...\n");

//     // 接受客户端连接
//     while ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) >= 0) {
//         printf("客户端已连接\n");
//         handle_client(client_socket);
//     }

//     if (client_socket < 0) {
//         perror("接受连接失败");
//     }

//     close(server_socket);
//     return 0;
// }