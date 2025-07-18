#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#define WIN32_LEAN_AND_MEAN
// #define _WIN32_WINNT 0x0600 // ȷ�� Windows Vista ����߰汾
#include <winsock2.h>
#include <ws2tcpip.h>              // ���ͷ�ļ����� inet_pton ������
<<<<<<< Updated upstream
#pragma comment(lib, "ws2_32.lib") // ���� Winsock ��
=======
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
>>>>>>> Stashed changes

static SOCKET sockfd = INVALID_SOCKET;
// ��������С
#define BUFFER_SIZE 1024

// ��ʼ��Windows Socket
static int init_winsock()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartupʧ��\n");
        return -1;
    }
    return 0;
}

// ���ӵ�������
int connect_to_server(const char *ip, int port)
{
    struct sockaddr_in server_addr;

    // ��ʼ��WinSock
    if (init_winsock() != 0)
    {
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET)
    {
        printf("�����׽���ʧ��: %d\n", WSAGetLastError());
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
        printf("IP��ַ��Ч\n");
        closesocket(sockfd);
        WSACleanup();
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("���ӷ�����ʧ��: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return -1;
    }

    return 0;
}

// �����ļ�����
void send_file(const char *file_path)
{
    FILE *file = fopen(file_path, "rb");
    if (!file)
    {
        printf("���ļ�ʧ��\n");
        return;
    }

    // ��ȡ�ļ��� - ͬʱ����Windows��Unix����·��
    const char *filename = strrchr(file_path, '/');
    if (!filename)
    {
        filename = strrchr(file_path, '\\');
    }
    filename = filename ? filename + 1 : file_path;
    uint16_t name_len = strlen(filename);

    // ��ȡ�ļ����ݳ���
    fseek(file, 0, SEEK_END);
    uint32_t content_len = ftell(file);
    fseek(file, 0, SEEK_SET);

    // �����ͷ���ļ���
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

    // ���Ͱ�ͷ���ļ���
    if (send(sockfd, header, header_len, 0) != header_len)
    {
        printf("�����ļ���ͷʧ��: %d\n", WSAGetLastError());
        fclose(file);
        closesocket(sockfd);
        WSACleanup();
        return;
    }

    // �����ļ�����
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        if (send(sockfd, buffer, bytes_read, 0) != bytes_read)
        {
            printf("�����ļ�����ʧ��: %d\n", WSAGetLastError());
            break;
        }
    }

    fclose(file);
    closesocket(sockfd);
    WSACleanup();
}

// �����ı�����
void send_text(const char *text)
{
    char type = 2;
    uint32_t text_len = strlen(text);
    uint32_t text_len_net = htonl(text_len);

    // ���Ͱ�ͷ
    if (send(sockfd, &type, 1, 0) != 1)
    {
        printf("�����ı�����ʧ��: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return;
    }

    if (send(sockfd, (const char *)&text_len_net, 4, 0) != 4)
    {
        printf("�����ı�����ʧ��: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return;
    }

    // �����ı�����
    size_t sent = 0;
    while (sent < text_len)
    {
        int n = send(sockfd, text + sent, text_len - sent, 0);
        if (n <= 0)
        {
            printf("�����ı�����ʧ��: %d\n", WSAGetLastError());
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
    // 1. ���ͼ�����֤��
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
        perror("������֤����ʧ��"); closesocket(sockfd); WSACleanup(); return -1;
    }
    if (send(sockfd, (const char *)&enc_auth_len_net, 4, 0) != 4) {
        perror("������֤����ʧ��"); closesocket(sockfd); WSACleanup(); return -1;
    }
    if (send(sockfd, (const char *)enc_auth, enc_auth_len, 0) != enc_auth_len) {
        perror("������֤����ʧ��"); closesocket(sockfd); WSACleanup(); return -1;
    }
    char auth_result;
    if (recv(sockfd, (char *)&auth_result, 1, 0) != 1) {
        perror("������֤���ʧ��"); closesocket(sockfd); WSACleanup(); return -1;
    }
    if (auth_result != 0) {
        fprintf(stderr, "��֤ʧ��\n"); closesocket(sockfd); WSACleanup(); return -1;
    }
    // 2. ���ͼ��������
    char type = 3;
    int cmd_len = strlen(command);
    unsigned char enc_cmd[512];
    int enc_cmd_len = aes_encrypt((unsigned char*)command, cmd_len, enc_cmd);
    uint32_t enc_cmd_len_net = htonl(enc_cmd_len);
    if (send(sockfd, &type, 1, 0) != 1) {
        perror("������������ʧ��"); closesocket(sockfd); WSACleanup(); return -1;
    }
    if (send(sockfd, (const char *)&enc_cmd_len_net, 4, 0) != 4) {
        perror("���������ʧ��"); closesocket(sockfd); WSACleanup(); return -1;
    }
    if (send(sockfd, (const char *)enc_cmd, enc_cmd_len, 0) != enc_cmd_len) {
        perror("������������ʧ��"); closesocket(sockfd); WSACleanup(); return -1;
    }
    // 3. ���ս������ԭЭ��һ�£�
    uint32_t result_len_net;
    if (recv(sockfd, (char *)&result_len_net, 4, 0) != 4) {
        perror("���ս������ʧ��"); closesocket(sockfd); WSACleanup(); return -1;
    }
    uint32_t result_len = ntohl(result_len_net);
    char buffer[1024];
    uint32_t total_received = 0;
    while (total_received < result_len) {
        size_t to_read = result_len - total_received;
        if (to_read > 1023) to_read = 1023;
        int n = recv(sockfd, buffer, to_read, 0);
        if (n <= 0) { perror("����ִ�н��ʧ��"); closesocket(sockfd); WSACleanup(); return -1; }
        buffer[n] = '\0';
        printf("%s", buffer);
        total_received += n;
    }
    char status;
    if (recv(sockfd, &status, 1, 0) != 1) {
        perror("����ִ��״̬ʧ��"); closesocket(sockfd); WSACleanup(); return -1;
    }
    closesocket(sockfd); WSACleanup();
    return status == 0 ? 0 : -1;
>>>>>>> Stashed changes
}