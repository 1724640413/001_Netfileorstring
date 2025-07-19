#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include "utils.h"

#define PORT 42323
#define BUFFER_SIZE 1024
#define FILE_DIR "/usr/local/bin/netfileorstring-server/file"
#define TEXT_FILE "/usr/local/bin/netfileorstring-server/file/001_data.txt"

// 判断文件夹是否存在，不存在则创建
void ensure_file_dir()
{
    struct stat st = {0};
    if (stat(FILE_DIR, &st) == -1)
    {
        mkdir(FILE_DIR, 0755);
    }
}

static const unsigned char aes_key[16] = "1234567890abcdef";
static const unsigned char aes_iv[16]  = "abcdef1234567890";

int aes_decrypt(const unsigned char *in, int in_len, unsigned char *out) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int out_len1 = 0, out_len2 = 0;
    EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, aes_key, aes_iv);
    EVP_DecryptUpdate(ctx, out, &out_len1, in, in_len);
    EVP_DecryptFinal_ex(ctx, out + out_len1, &out_len2);
    EVP_CIPHER_CTX_free(ctx);
    return out_len1 + out_len2;
}

// 优化后的 handle_client，支持大文件边收边写，防止缓冲区溢出
void handle_client_data(int client_socket)
{
    char recv_buffer[BUFFER_SIZE];
    char data_buffer[BUFFER_SIZE * 4];
    int data_len = 0;
    int authed = 0; // 认证标志
    ensure_file_dir();
    while (1)
    {
        int bytes_received = recv(client_socket, recv_buffer, sizeof(recv_buffer), 0);
        if (bytes_received == 0) {
            printf("数据传输完成，客户端关闭连接\n");
            ERR_LOG("数据传输完成，客户端关闭连接");
            break;
        }
        if (bytes_received < 0) {
            perror("接收数据失败");
            ERR_LOG("接收数据失败");
            break;
        }
        if (data_len + bytes_received > sizeof(data_buffer))
        {
            fprintf(stderr, "接收缓冲区溢出\n");
            ERR_LOG("接收缓冲区溢出");
            break;
        }
        memcpy(data_buffer + data_len, recv_buffer, bytes_received);
        data_len += bytes_received;
        int offset = 0;
        while (offset < data_len)
        {
            if (data_len - offset < 1)
                break;
            char type = data_buffer[offset];
            int pkg_len = 0;
            if (type == 10) // 认证包
            {
                if (data_len - offset < 5)
                    break;
                uint32_t enc_auth_len;
                memcpy(&enc_auth_len, data_buffer + offset + 1, 4);
                enc_auth_len = ntohl(enc_auth_len);
                if (data_len - offset < 5 + enc_auth_len)
                    break;
                unsigned char enc_auth[256] = {0};
                memcpy(enc_auth, data_buffer + offset + 5, enc_auth_len);
                unsigned char auth_info[128] = {0};
                aes_decrypt(enc_auth, enc_auth_len, auth_info);
                // 简单用户名密码校验
                if (strcmp((char*)auth_info, "admin:123456") == 0) {
                    char result = 0; // 认证成功
                    send(client_socket, &result, 1, 0);
                    authed = 1;
                } else {
                    char result = 1; // 认证失败
                    send(client_socket, &result, 1, 0);
                    close(client_socket);
                    return;
                }
                pkg_len = 5 + enc_auth_len;
            }
            else if (type == 3) // 命令包
            {
                if (!authed) {
                    printf("未认证，拒绝执行命令\n");
                    close(client_socket);
                    return;
                }
                if (data_len - offset < 5)
                    break;
                uint32_t enc_cmd_len;
                memcpy(&enc_cmd_len, data_buffer + offset + 1, 4);
                enc_cmd_len = ntohl(enc_cmd_len);
                if (data_len - offset < 5 + enc_cmd_len)
                    break;
                unsigned char enc_cmd[512] = {0};
                memcpy(enc_cmd, data_buffer + offset + 5, enc_cmd_len);
                unsigned char cmd[256] = {0};
                int dec_len = aes_decrypt(enc_cmd, enc_cmd_len, cmd);
                // 保证字符串结尾，防止脏数据
                cmd[dec_len < sizeof(cmd) ? dec_len : sizeof(cmd) - 1] = '\0';
                // 执行命令
                FILE *fp = popen((char*)cmd, "r");
                if (!fp) {
                    perror("命令执行失败");
                    char status = 1;
                    uint32_t zero = 0;
                    send(client_socket, &zero, 4, 0);
                    send(client_socket, &status, 1, 0);
                    close(client_socket);
                    return;
                }
                char result[BUFFER_SIZE * 4] = {0};
                size_t n = fread(result, 1, sizeof(result) - 1, fp);
                pclose(fp);
                uint32_t result_len = n;
                uint32_t result_len_net = htonl(result_len);
                send(client_socket, &result_len_net, 4, 0);
                send(client_socket, result, result_len, 0);
                char status = 0;
                send(client_socket, &status, 1, 0);
                pkg_len = 5 + enc_cmd_len;
                close(client_socket);
                return;
            }
            else if (type == 1)
            {
                // 文件包头部
                if (data_len - offset < 7)
                    break;
                uint16_t name_len;
                memcpy(&name_len, data_buffer + offset + 1, 2);
                name_len = ntohs(name_len);
                uint32_t content_len;
                memcpy(&content_len, data_buffer + offset + 3, 4);
                content_len = ntohl(content_len);

                if (data_len - offset < 7 + name_len)
                    break;

                // 文件名
                char filename[256] = {0};
                memcpy(filename, data_buffer + offset + 7, name_len);
                filename[name_len] = '\0';

                char filepath[512];
                snprintf(filepath, sizeof(filepath), "%s/%s", FILE_DIR, filename);

                size_t header_and_name = 7 + name_len;
                size_t already_in_buffer = data_len - (offset + header_and_name);
                size_t to_write = already_in_buffer > content_len ? content_len : already_in_buffer;

                // 边收边写
                FILE *fp = fopen(filepath, "wb");
                if (!fp)
                {
                    perror("无法写入文件");
                    ERR_LOG("无法写入文件: %s", filepath);
                    // 跳过此包
                    offset += header_and_name + content_len;
                    continue;
                }

                // 先写缓冲区已有内容
                if (to_write > 0)
                    fwrite(data_buffer + offset + header_and_name, 1, to_write, fp);

                size_t remain = content_len - to_write;
                while (remain > 0)
                {
                    int n = recv(client_socket, recv_buffer, remain < sizeof(recv_buffer) ? remain : sizeof(recv_buffer), 0);
                    if (n <= 0)
                        break;
                    fwrite(recv_buffer, 1, n, fp);
                    remain -= n;
                }
                fclose(fp);
                // printf("已保存文件: %s\n", filepath);
                ERR_LOG("已保存文件: %s", filepath);

                pkg_len = header_and_name + content_len;
            }
            else if (type == 2)
            {
                if (data_len - offset < 5)
                    break;
                uint32_t text_len;
                memcpy(&text_len, data_buffer + offset + 1, 4);
                text_len = ntohl(text_len);
                if (data_len - offset < 5 + text_len)
                    break;
                pkg_len = 5 + text_len;

                FILE *fp = fopen(TEXT_FILE, "ab");
                if (fp)
                {
                    fwrite(data_buffer + offset + 5, 1, text_len, fp);
                    fwrite("\n", 1, 1, fp);
                    fclose(fp);
                    //printf("已保存文本到: %s\n", TEXT_FILE);
                    ERR_LOG("已保存文本到: %s", TEXT_FILE);
                }
                else
                {
                    perror("无法写入文本文件");
                    ERR_LOG("无法写入文本文件: %s", TEXT_FILE);
                }
            }
            else
            {
                printf("未知数据类型: %d\n", type);
                break;
            }

            offset += pkg_len;
        }

        if (offset < data_len)
        {
            memmove(data_buffer, data_buffer + offset, data_len - offset);
            data_len -= offset;
        }
        else
        {
            data_len = 0;
        }
    }

    close(client_socket);
}
