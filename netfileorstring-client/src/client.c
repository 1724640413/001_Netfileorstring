#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

static int sockfd = -1;
// 缓存区大小
#define BUFFER_SIZE 1024

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

    // 获取文件名
    const char *filename = strrchr(file_path, '/');
    filename = filename ? filename + 1 : file_path;
    uint16_t name_len = strlen(filename);

    // 获取文件内容长度
    fseek(file, 0, SEEK_END);
    uint32_t content_len = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 构造包头和文件名
    char header[1 + 2 + 4 + 256]; // type + name_len + content_len + filename
    int header_len = 0;
    header[0] = 1; // type
    header_len += 1;
    uint16_t name_len_net = htons(name_len);
    memcpy(header + header_len, &name_len_net, 2);
    header_len += 2;
    uint32_t content_len_net = htonl(content_len);
    memcpy(header + header_len, &content_len_net, 4);
    header_len += 4;
    memcpy(header + header_len, filename, name_len);
    header_len += name_len;

    // 发送包头和文件名
    if (send(sockfd, header, header_len, 0) != header_len) {
        perror("发送文件包头失败");
        fclose(file);
        close(sockfd); // 发送失败时关闭sockfd
        return;
    }

    // 发送文件内容
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (send(sockfd, buffer, bytes_read, 0) != bytes_read) {
            perror("发送文件内容失败");
            break;
        }
    }

    fclose(file);
    // 等待服务器响应
    char response;
    if (recv(sockfd, &response, 1, 0) != 1) 
    {
        perror("接收服务器响应失败");
    } 
    else if (response == 0) 
    {
        printf("文件发送成功\n");
        close(sockfd); // 数据发送完成后关闭sockfd
    } 
    else 
    {
        printf("文件发送失败\n");
        close(sockfd); // 数据发送失败时关闭sockfd
    }
}

// 发送文本内容
void send_text(const char *text) {
    char type = 2;
    uint32_t text_len = strlen(text);
    uint32_t text_len_net = htonl(text_len);

    // 发送包头
    if (send(sockfd, &type, 1, 0) != 1) {
        perror("发送文本类型失败");
        close(sockfd);
        return;
    }
    if (send(sockfd, &text_len_net, 4, 0) != 4) {
        perror("发送文本长度失败");
        close(sockfd);
        return;
    }

    // 发送文本内容
    size_t sent = 0;
    while (sent < text_len) {
        int n = send(sockfd, text + sent, text_len - sent, 0);
        if (n <= 0) {
            perror("发送文本内容失败");
            break;
        }
        sent += n;
    }

    close(sockfd); // 数据发送完成后关闭sockfd
}

// 执行远程命令并获取结果
int execute_remote_command(const char *command) {
    char type = 3;  // 定义新的类型：3表示远程命令
    uint32_t cmd_len = strlen(command);
    uint32_t cmd_len_net = htonl(cmd_len);

    // 发送命令类型
    if (send(sockfd, &type, 1, 0) != 1) {
        perror("发送命令类型失败");
        close(sockfd);
        return -1;
    }

    // 发送命令长度
    if (send(sockfd, &cmd_len_net, 4, 0) != 4) {
        perror("发送命令长度失败");
        close(sockfd);
        return -1;
    }

    // 发送命令内容
    if (send(sockfd, command, cmd_len, 0) != cmd_len) {
        perror("发送命令内容失败");
        close(sockfd);
        return -1;
    }

    // 接收结果长度
    uint32_t result_len_net;
    if (recv(sockfd, &result_len_net, 4, 0) != 4) {
        perror("接收结果长度失败");
        close(sockfd);
        return -1;
    }
    uint32_t result_len = ntohl(result_len_net);

    // 接收执行结果
    char buffer[BUFFER_SIZE];
    uint32_t total_received = 0;
    
    while (total_received < result_len) {
        size_t to_read = result_len - total_received;
        if (to_read > BUFFER_SIZE - 1) {
            to_read = BUFFER_SIZE - 1;
        }
        
        int n = recv(sockfd, buffer, to_read, 0);
        if (n <= 0) {
            perror("接收执行结果失败");
            close(sockfd);
            return -1;
        }
        
        buffer[n] = '\0';
        printf("%s", buffer);
        total_received += n;
    }

    // 接收执行状态
    char status;
    if (recv(sockfd, &status, 1, 0) != 1) {
        perror("接收执行状态失败");
        close(sockfd);
        return -1;
    }

    close(sockfd);
    return status == 0 ? 0 : -1;
}