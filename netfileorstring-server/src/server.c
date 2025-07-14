#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>
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

// 优化后的 handle_client，支持大文件边收边写，防止缓冲区溢出
void handle_client_data(int client_socket)
{
    char recv_buffer[BUFFER_SIZE];
    char data_buffer[BUFFER_SIZE * 4];
    int data_len = 0;

    ensure_file_dir();

    while (1)
    {
        int bytes_received = recv(client_socket, recv_buffer, sizeof(recv_buffer), 0);
        if (bytes_received == 0) {
            // 客户端正常关闭连接
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

            if (type == 1)
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
