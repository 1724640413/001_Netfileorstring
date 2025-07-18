#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#define WIN32_LEAN_AND_MEAN
// #define _WIN32_WINNT 0x0600 // 确保 Windows Vista 或更高版本
#include <winsock2.h>
#include <ws2tcpip.h>              // 这个头文件包含 inet_pton 的声明
<<<<<<< Updated upstream
#pragma comment(lib, "ws2_32.lib") // 链接 Winsock 库
=======
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
>>>>>>> Stashed changes

static SOCKET sockfd = INVALID_SOCKET;
// 缓存区大小
#define BUFFER_SIZE 1024

// 初始化Windows Socket
static int init_winsock()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup失败\n");
        return -1;
    }
    return 0;
}

// 连接到服务器
int connect_to_server(const char *ip, int port)
{
    struct sockaddr_in server_addr;

    // 初始化WinSock
    if (init_winsock() != 0)
    {
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET)
    {
        printf("创建套接字失败: %d\n", WSAGetLastError());
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
        printf("IP地址无效\n");
        closesocket(sockfd);
        WSACleanup();
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("连接服务器失败: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return -1;
    }

    return 0;
}

// 发送文件内容
void send_file(const char *file_path)
{
    FILE *file = fopen(file_path, "rb");
    if (!file)
    {
        printf("打开文件失败\n");
        return;
    }

    // 获取文件名 - 同时处理Windows和Unix风格的路径
    const char *filename = strrchr(file_path, '/');
    if (!filename)
    {
        filename = strrchr(file_path, '\\');
    }
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
    if (send(sockfd, header, header_len, 0) != header_len)
    {
        printf("发送文件包头失败: %d\n", WSAGetLastError());
        fclose(file);
        closesocket(sockfd);
        WSACleanup();
        return;
    }

    // 发送文件内容
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        if (send(sockfd, buffer, bytes_read, 0) != bytes_read)
        {
            printf("发送文件内容失败: %d\n", WSAGetLastError());
            break;
        }
    }

    fclose(file);
    closesocket(sockfd);
    WSACleanup();
}

// 发送文本内容
void send_text(const char *text)
{
    char type = 2;
    uint32_t text_len = strlen(text);
    uint32_t text_len_net = htonl(text_len);

    // 发送包头
    if (send(sockfd, &type, 1, 0) != 1)
    {
        printf("发送文本类型失败: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return;
    }

    if (send(sockfd, (const char *)&text_len_net, 4, 0) != 4)
    {
        printf("发送文本长度失败: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return;
    }

    // 发送文本内容
    size_t sent = 0;
    while (sent < text_len)
    {
        int n = send(sockfd, text + sent, text_len - sent, 0);
        if (n <= 0)
        {
            printf("发送文本内容失败: %d\n", WSAGetLastError());
            break;
        }
        sent += n;
    }

    closesocket(sockfd);
    WSACleanup();
<<<<<<< Updated upstream
=======
}

int execute_remote_command(const char *command) {
    // 1. 发送加密认证包
    char auth_type = 10;
    const char *username = "admin";
    const char *password = "123456";
    char auth_info[128];
    snprintf(auth_info, sizeof(auth_info), "%s:%s", username, password);
    int auth_len = strlen(auth_info);
    unsigned char enc_auth[256];
    int enc_auth_len = aes_encrypt((unsigned char*)auth_info, auth_len, enc_auth);
    uint32_t enc_auth_len_net = htonl(enc_auth_len);
    if (send(sockfd, &auth_type, 1, 0) != 1) {
        perror("发送认证类型失败"); closesocket(sockfd); WSACleanup(); return -1;
    }
    if (send(sockfd, (const char *)&enc_auth_len_net, 4, 0) != 4) {
        perror("发送认证长度失败"); closesocket(sockfd); WSACleanup(); return -1;
    }
    if (send(sockfd, (const char *)enc_auth, enc_auth_len, 0) != enc_auth_len) {
        perror("发送认证内容失败"); closesocket(sockfd); WSACleanup(); return -1;
    }
    char auth_result;
    if (recv(sockfd, (char *)&auth_result, 1, 0) != 1) {
        perror("接收认证结果失败"); closesocket(sockfd); WSACleanup(); return -1;
    }
    if (auth_result != 0) {
        fprintf(stderr, "认证失败\n"); closesocket(sockfd); WSACleanup(); return -1;
    }
    // 2. 发送加密命令包
    char type = 3;
    int cmd_len = strlen(command);
    unsigned char enc_cmd[512];
    int enc_cmd_len = aes_encrypt((unsigned char*)command, cmd_len, enc_cmd);
    uint32_t enc_cmd_len_net = htonl(enc_cmd_len);
    if (send(sockfd, &type, 1, 0) != 1) {
        perror("发送命令类型失败"); closesocket(sockfd); WSACleanup(); return -1;
    }
    if (send(sockfd, (const char *)&enc_cmd_len_net, 4, 0) != 4) {
        perror("发送命令长度失败"); closesocket(sockfd); WSACleanup(); return -1;
    }
    if (send(sockfd, (const char *)enc_cmd, enc_cmd_len, 0) != enc_cmd_len) {
        perror("发送命令内容失败"); closesocket(sockfd); WSACleanup(); return -1;
    }
    // 3. 接收结果（与原协议一致）
    uint32_t result_len_net;
    if (recv(sockfd, (char *)&result_len_net, 4, 0) != 4) {
        perror("接收结果长度失败"); closesocket(sockfd); WSACleanup(); return -1;
    }
    uint32_t result_len = ntohl(result_len_net);
    char buffer[1024];
    uint32_t total_received = 0;
    while (total_received < result_len) {
        size_t to_read = result_len - total_received;
        if (to_read > 1023) to_read = 1023;
        int n = recv(sockfd, buffer, to_read, 0);
        if (n <= 0) { perror("接收执行结果失败"); closesocket(sockfd); WSACleanup(); return -1; }
        buffer[n] = '\0';
        printf("%s", buffer);
        total_received += n;
    }
    char status;
    if (recv(sockfd, &status, 1, 0) != 1) {
        perror("接收执行状态失败"); closesocket(sockfd); WSACleanup(); return -1;
    }
    closesocket(sockfd); WSACleanup();
    return status == 0 ? 0 : -1;
>>>>>>> Stashed changes
}