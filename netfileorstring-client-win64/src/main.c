#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"

void usage(const char *prog) {
    printf("�÷�: %s <ip> <port> <file|text> <����>\n", prog);
    printf("ʾ�������ļ�: %s 127.0.0.1 8888 file ./test.txt\n", prog);
    printf("ʾ�������ı�: %s 127.0.0.1 8888 text \"��ã�����\"\n", prog);
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
        fprintf(stderr, "���ӷ�����ʧ��\n");
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
    
    printf("�������\n");
    return 0;
}