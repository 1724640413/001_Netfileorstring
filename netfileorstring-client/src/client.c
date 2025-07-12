#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

static int sockfd = -1;

// 连接到服务器
int connect_to_server(const char *ip, int port) {
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("创建套接字失败");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("IP地址无效");
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("连接服务器失败");
        close(sockfd);
        return -1;
    }

    return 0;
}

// 发送文件内容
void send_file(const char *file_path) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("打开文件失败");
        return;
    }

    char buffer[1024];
    size_t bytes_read;

    // 发送文件标识
    char type = 1;
    send(sockfd, &type, 1, 0);

    // 发送文件名长度和文件名
    const char *filename = strrchr(file_path, '/');
    filename = filename ? filename + 1 : file_path;
    uint16_t name_len = strlen(filename);
    uint16_t name_len_net = htons(name_len);
    send(sockfd, &name_len_net, sizeof(name_len_net), 0);
    send(sockfd, filename, name_len, 0);

    // 发送文件内容
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(sockfd, buffer, bytes_read, 0);
    }

    fclose(file);
    close(sockfd);
}

// 发送文本内容
void send_text(const char *text) {
    // 发送文本标识
    char type = 2;
    send(sockfd, &type, 1, 0);

    // 发送文本长度
    uint32_t text_len = strlen(text);
    uint32_t text_len_net = htonl(text_len);
    send(sockfd, &text_len_net, sizeof(text_len_net), 0);

    // 发送文本内容
    send(sockfd, text, text_len, 0);

    close(sockfd);
}