// rsh_cli.c
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "dshlib.h"
#include "rshlib.h"

int start_client(char *server_ip, int port) {
    int cli_socket;
    struct sockaddr_in addr;
    int ret;
    
    cli_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (cli_socket < 0) {
        perror("socket");
        return ERR_RDSH_CLIENT;
    }
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(server_ip);
    
    ret = connect(cli_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("connect");
        close(cli_socket);
        return ERR_RDSH_CLIENT;
    }
    return cli_socket;
}

int client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc) {
    if(cli_socket > 0)
        close(cli_socket);
    free(cmd_buff);
    free(rsp_buff);
    return rc;
}

int exec_remote_cmd_loop(char *address, int port) {
    char *cmd_buff = malloc(RDSH_COMM_BUFF_SZ);
    char *rsp_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (cmd_buff == NULL || rsp_buff == NULL) {
        return client_cleanup(-1, cmd_buff, rsp_buff, ERR_MEMORY);
    }
    
    int cli_socket = start_client(address, port);
    if (cli_socket < 0) {
        perror("start client");
        return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_CLIENT);
    }
    
    while (1) {
        printf("rsh> ");
        if (fgets(cmd_buff, RDSH_COMM_BUFF_SZ, stdin) == NULL) {
            printf("\n");
            break;
        }
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
        
        if (strcmp(cmd_buff, "exit") == 0 || strcmp(cmd_buff, "stop-server") == 0) {
            if (send(cli_socket, cmd_buff, strlen(cmd_buff) + 1, 0) == -1)
                perror("send");
            break;
        }
        
        if (send(cli_socket, cmd_buff, strlen(cmd_buff) + 1, 0) == -1) {
            perror("send");
            break;
        }
        
        int total_received = 0;
        bool eof_received = false;
        memset(rsp_buff, 0, RDSH_COMM_BUFF_SZ);
        while (!eof_received) {
            int recv_bytes = recv(cli_socket, rsp_buff + total_received, RDSH_COMM_BUFF_SZ - total_received, 0);
            if (recv_bytes < 0) {
                perror("recv");
                client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_COMMUNICATION);
                return ERR_RDSH_COMMUNICATION;
            } else if (recv_bytes == 0) {
                eof_received = true;
                break;
            }
            total_received += recv_bytes;
            if (rsp_buff[total_received - 1] == RDSH_EOF_CHAR)
                eof_received = true;
        }
        if (total_received > 0 && rsp_buff[total_received - 1] == RDSH_EOF_CHAR)
            rsp_buff[total_received - 1] = '\0';
        
        printf("%s\n", rsp_buff);
    }
    
    return client_cleanup(cli_socket, cmd_buff, rsp_buff, OK);
}
