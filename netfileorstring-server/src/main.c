#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "server.h"
#include "utils.h"

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

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(new_socket);
            continue;
        } else if (pid == 0) {
            // 子进程
            close(server_fd); // 子进程不需要监听socket
            printf("进入 handle_client_data\n");
            handle_client_data(new_socket);
            printf("退出 handle_client_data\n");
            exit(0);
        } else {
            // 父进程
            close(new_socket); // 父进程不处理该连接
        }
    }

    return 0;
}