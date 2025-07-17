#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"

void usage(const char *prog) {
    printf("用法: %s <ip> <port> <file|text> <内容>\n", prog);
    printf("示例发送文件: %s 127.0.0.1 8888 file ./test.txt\n", prog);
    printf("示例发送文本: %s 127.0.0.1 8888 text \"你好，世界\"\n", prog);
    printf("示例执行命令: %s 127.0.0.1 8888 cmd \"ls -l\"\n", prog);
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        usage(argv[0]);
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);
    const char *mode = argv[3];
    const char *content = argv[4];

    if (connect_to_server(ip, port) != 0) 
    {
        fprintf(stderr, "连接服务器失败\n");
        return 1;
    }

    if (strcmp(mode, "file") == 0) 
    {
        send_file(content);
    }
    else if (strcmp(mode, "text") == 0) 
    {
        send_text(content);
    }
    else if (strcmp(mode, "cmd") == 0) 
    {
        return execute_remote_command(content);
    }
    else
    {
        usage(argv[0]);
        return 1;
    }
    
    

    printf("发送完成\n");
    return 0;
}