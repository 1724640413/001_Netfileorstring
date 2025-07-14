#ifndef CLIENT_H
#define CLIENT_H

// 连接到服务器，成功返回0，失败返回-1
int connect_to_server(const char *ip, int port);

// 发送文件内容到服务器
void send_file(const char *file_path);

// 发送文本内容到服务器
void send_text(const char *text);

#endif // CLIENT_H