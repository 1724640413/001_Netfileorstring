#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 42323
#define BUFFER_SIZE 1024
#define FILE_DIR "file"
#define TEXT_FILE "file/000_text.txt"

// 判断文件夹是否存在，不存在则创建
void ensure_file_dir() {
    struct stat st = {0};
    if (stat(FILE_DIR, &st) == -1) {
        mkdir(FILE_DIR, 0755);
    }
}

// 处理客户端数据
void handle_client_data(const char *buffer, int length) {
    ensure_file_dir();

    if (length < 1) return;

    char type = buffer[0];
    if (type == 1) { // 文件
        if (length < 3) return;
        // 获取文件名长度
        uint16_t name_len;
        memcpy(&name_len, buffer + 1, 2);
        name_len = ntohs(name_len);
        if (length < 3 + name_len) return;

        // 获取文件名
        char filename[256] = {0};
        memcpy(filename, buffer + 3, name_len);
        filename[name_len] = '\0';

        // 构造完整路径
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", FILE_DIR, filename);

        // 写入文件内容
        FILE *fp = fopen(filepath, "wb");
        if (fp) {
            fwrite(buffer + 3 + name_len, 1, length - 3 - name_len, fp);
            fclose(fp);
            printf("已保存文件: %s\n", filepath);
        } else {
            perror("无法写入文件");
        }
    } else if (type == 2) { // 文本
        if (length < 5) return;
        uint32_t text_len;
        memcpy(&text_len, buffer + 1, 4);
        text_len = ntohl(text_len);
        if (length < 5 + text_len) return;

        // 检查文本文件是否存在，不存在则创建
        FILE *fp = fopen(TEXT_FILE, "ab");
        if (fp) {
            fwrite(buffer + 5, 1, text_len, fp);
            fwrite("\n", 1, 1, fp);
            fclose(fp);
            printf("已保存文本到: %s\n", TEXT_FILE);
        } else {
            perror("无法写入文本文件");
        }
    } else {
        printf("未知数据类型: %d\n", type);
    }
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // 接收客户端发送的数据
    bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0) {
        perror("接收数据失败");
        close(client_socket);
        return;
    }

    buffer[bytes_received] = '\0'; // 确保字符串以null结尾
    printf("接收到的数据: %s\n", buffer);

    // 处理接收到的数据
    handle_client_data(buffer, bytes_received);

    close(client_socket);
}
