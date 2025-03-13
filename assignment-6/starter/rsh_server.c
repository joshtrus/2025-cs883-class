// rsh_server.c
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <sys/select.h>
#include "dshlib.h"
#include "rshlib.h"
#include <stdint.h>

volatile int server_stop = 0;
int rc_last = 0;

/*--------------------------------------------------------------
   boot_server(ifaces, port)
--------------------------------------------------------------*/
int boot_server(char *ifaces, int port) {
    int svr_socket;
    int ret;
    struct sockaddr_in addr;
    int enable = 1;
    
    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket < 0) {
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }
    
    ret = setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    if (ret < 0) {
        perror("setsockopt");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ifaces);
    
    ret = bind(svr_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("bind");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }
    
    ret = listen(svr_socket, 20);
    if (ret < 0) {
        perror("listen");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }
    
    return svr_socket;
}

/*--------------------------------------------------------------
   stop_server(svr_socket)
--------------------------------------------------------------*/
int stop_server(int svr_socket) {
    return close(svr_socket);
}

/*--------------------------------------------------------------
   send_message_eof(cli_sock)
--------------------------------------------------------------*/
int send_message_eof(int cli_sock) {
    int send_len = (int)sizeof(RDSH_EOF_CHAR);
    int sent_len = send(cli_sock, &RDSH_EOF_CHAR, send_len, 0);
    if (sent_len != send_len)
        return ERR_RDSH_COMMUNICATION;
    return OK;
}

/*--------------------------------------------------------------
   send_message_string(cli_sock, buff)
--------------------------------------------------------------*/
int send_message_string(int cli_sock, char *buff) {
    int len = strlen(buff) + 1;
    int sent = send(cli_sock, buff, len, 0);
    if (sent != len) {
        fprintf(stderr, CMD_ERR_RDSH_SEND, sent, len);
        return ERR_RDSH_COMMUNICATION;
    }
    return send_message_eof(cli_sock);
}

/*--------------------------------------------------------------
   Built-In Command Functions
--------------------------------------------------------------*/
Built_In_Cmds rsh_match_command(const char *input) {
    if (strcmp(input, "exit") == 0)
        return BI_CMD_EXIT;
    if (strcmp(input, "dragon") == 0)
        return BI_CMD_DRAGON;
    if (strcmp(input, "cd") == 0)
        return BI_CMD_CD;
    if (strcmp(input, "stop-server") == 0)
        return BI_CMD_STOP_SVR;
    if (strcmp(input, "rc") == 0)
        return BI_CMD_RC;
    return BI_NOT_BI;
}

Built_In_Cmds rsh_exec_built_in_cmd(int cli_sock, cmd_buff_t *cmd) {
    Built_In_Cmds ctype = rsh_match_command(cmd->argv[0]);
    char buf[256];

    switch (ctype) {
        case BI_CMD_EXIT:
            return BI_CMD_EXIT;
        case BI_CMD_STOP_SVR:
            return BI_CMD_STOP_SVR;
        case BI_CMD_RC:
            snprintf(buf, sizeof(buf), "rdsh-exec: rc = %d\n", rc_last);
            send_message_string(cli_sock, buf);
            send_message_eof(cli_sock);
            return BI_EXECUTED;
        case BI_CMD_CD:
            if (cmd->argc < 2) {
                snprintf(buf, sizeof(buf), "cd error: missing argument\n");
                rc_last = 1;
                send_message_string(cli_sock, buf);
            } else {
                if (chdir(cmd->argv[1]) != 0) {
                    rc_last = errno;
                    snprintf(buf, sizeof(buf), "cd error: %s\n", strerror(errno));
                    send_message_string(cli_sock, buf);
                } else {
                    rc_last = 0;
                    snprintf(buf, sizeof(buf), "Changed directory to %s\n", cmd->argv[1]);
                    send_message_string(cli_sock, buf);
                }
            }
            send_message_eof(cli_sock);
            return BI_EXECUTED;
        default:
            return BI_NOT_BI;
    }
}

/*--------------------------------------------------------------
   remote_exec_cmd_with_redirection(cli_sock, cmd)
--------------------------------------------------------------*/
int remote_exec_cmd_with_redirection(int cli_sock, cmd_buff_t *cmd) {
    pid_t pid = fork();
    if (pid < 0) {
         char err_buf[256];
         snprintf(err_buf, sizeof(err_buf), "fork error: %s\n", strerror(errno));
         send_message_string(cli_sock, err_buf);
         return ERR_EXEC_CMD;
    } else if (pid == 0) {
         // Child process:
         if (cmd->input_file != NULL) {
             int fd_in = open(cmd->input_file, O_RDONLY);
             if (fd_in < 0) {
                 char err_buf[256];
                 snprintf(err_buf, sizeof(err_buf), "open input_file error: %s\n", strerror(errno));
                 send_message_string(cli_sock, err_buf);
                 exit(errno);
             }
             if (dup2(fd_in, STDIN_FILENO) == -1) {
                 char err_buf[256];
                 snprintf(err_buf, sizeof(err_buf), "dup2 input_file -> STDIN error: %s\n", strerror(errno));
                 send_message_string(cli_sock, err_buf);
                 exit(errno);
             }
             close(fd_in);
         }
         if (cmd->output_file != NULL) {
             int flags = O_WRONLY | O_CREAT;
             flags |= cmd->append_mode ? O_APPEND : O_TRUNC;
             int fd_out = open(cmd->output_file, flags, 0644);
             if (fd_out < 0) {
                 char err_buf[256];
                 snprintf(err_buf, sizeof(err_buf), "open output_file error: %s\n", strerror(errno));
                 send_message_string(cli_sock, err_buf);
                 exit(errno);
             }
             if (dup2(fd_out, STDOUT_FILENO) == -1) {
                 char err_buf[256];
                 snprintf(err_buf, sizeof(err_buf), "dup2 output_file -> STDOUT error: %s\n", strerror(errno));
                 send_message_string(cli_sock, err_buf);
                 exit(errno);
             }
             close(fd_out);
         } else {
             if (dup2(cli_sock, STDOUT_FILENO) == -1) {
                 char err_buf[256];
                 snprintf(err_buf, sizeof(err_buf), "dup2 cli_sock -> STDOUT error: %s\n", strerror(errno));
                 send_message_string(cli_sock, err_buf);
                 exit(errno);
             }
             if (dup2(cli_sock, STDERR_FILENO) == -1) {
                 char err_buf[256];
                 snprintf(err_buf, sizeof(err_buf), "dup2 cli_sock -> STDERR error: %s\n", strerror(errno));
                 send_message_string(cli_sock, err_buf);
                 exit(errno);
             }
         }
         execvp(cmd->argv[0], cmd->argv);
         {
             char err_buf[256];
             snprintf(err_buf, sizeof(err_buf), "execvp error: %s\n", strerror(errno));
             send_message_string(cli_sock, err_buf);
         }
         exit(errno);
    } else {
         int status = 0;
         waitpid(pid, &status, 0);
         rc_last = WEXITSTATUS(status);
         send_message_eof(cli_sock);
         return rc_last;
    }
}

/*--------------------------------------------------------------
   rsh_execute_pipeline(cli_sock, clist)
--------------------------------------------------------------*/
int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    int num_cmds = clist->num;
    if (num_cmds == 0)
         return WARN_NO_CMDS;
    
    int pipes[num_cmds - 1][2];
    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            char err_buf[256];
            snprintf(err_buf, sizeof(err_buf), "pipe error: %s\n", strerror(errno));
            send_message_string(cli_sock, err_buf);
            return ERR_MEMORY;
        }
    }
    
    pid_t pids[CMD_MAX];
    for (int i = 0; i < num_cmds; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            char err_buf[256];
            snprintf(err_buf, sizeof(err_buf), "fork error: %s\n", strerror(errno));
            send_message_string(cli_sock, err_buf);
            return ERR_EXEC_CMD;
        } else if (pid == 0) {
            if (i == 0) {
                int null_fd = open("/dev/null", O_RDONLY);
                if (null_fd < 0) {
                    char err_buf[256];
                    snprintf(err_buf, sizeof(err_buf), "open /dev/null error: %s\n", strerror(errno));
                    send_message_string(cli_sock, err_buf);
                    exit(errno);
                }
                if (dup2(null_fd, STDIN_FILENO) == -1) {
                    char err_buf[256];
                    snprintf(err_buf, sizeof(err_buf), "dup2 /dev/null -> STDIN error: %s\n", strerror(errno));
                    send_message_string(cli_sock, err_buf);
                    exit(errno);
                }
                close(null_fd);
            } else {
                if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1) {
                    char err_buf[256];
                    snprintf(err_buf, sizeof(err_buf), "dup2 pipe -> STDIN error: %s\n", strerror(errno));
                    send_message_string(cli_sock, err_buf);
                    exit(errno);
                }
            }
            if (i == num_cmds - 1) {
                if (dup2(cli_sock, STDOUT_FILENO) == -1) {
                    char err_buf[256];
                    snprintf(err_buf, sizeof(err_buf), "dup2 cli_sock -> STDOUT error: %s\n", strerror(errno));
                    send_message_string(cli_sock, err_buf);
                    exit(errno);
                }
                if (dup2(cli_sock, STDERR_FILENO) == -1) {
                    char err_buf[256];
                    snprintf(err_buf, sizeof(err_buf), "dup2 cli_sock -> STDERR error: %s\n", strerror(errno));
                    send_message_string(cli_sock, err_buf);
                    exit(errno);
                }
            } else {
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                    char err_buf[256];
                    snprintf(err_buf, sizeof(err_buf), "dup2 pipe -> STDOUT error: %s\n", strerror(errno));
                    send_message_string(cli_sock, err_buf);
                    exit(errno);
                }
            }
            for (int j = 0; j < num_cmds - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            {
                char err_buf[256];
                snprintf(err_buf, sizeof(err_buf), "execvp error: %s\n", strerror(errno));
                send_message_string(cli_sock, err_buf);
            }
            exit(errno);
        } else {
            pids[i] = pid;
        }
    }
    for (int i = 0; i < num_cmds - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    int status = 0;
    for (int i = 0; i < num_cmds; i++) {
         waitpid(pids[i], &status, 0);
    }
    rc_last = WEXITSTATUS(status);
    send_message_eof(cli_sock);
    return rc_last;
}

/*--------------------------------------------------------------
   exec_client_requests(cli_sock)
--------------------------------------------------------------*/
int exec_client_requests(int cli_sock) {
    char *io_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (io_buff == NULL)
        return ERR_RDSH_SERVER;
    int rc = OK;
    while (1) {
        memset(io_buff, 0, RDSH_COMM_BUFF_SZ);
        int recv_bytes = recv(cli_sock, io_buff, RDSH_COMM_BUFF_SZ, 0);
        if (recv_bytes <= 0) {
            perror("recv");
            rc = ERR_RDSH_COMMUNICATION;
            break;
        }
        printf("rdsh-exec: %s\n", io_buff);
        
        // Handle "exit" and "stop-server" immediately.
        if (strcmp(io_buff, "exit") == 0) {
            rc = OK;
            break;
        } else if (strcmp(io_buff, "stop-server") == 0) {
            rc = OK_EXIT;
            server_stop = 1;
            break;
        }

        // Build a command buffer from the received input.
        cmd_buff_t cmd;
        if (alloc_cmd_buff(&cmd) != OK) {
            rc = ERR_MEMORY;
            break;
        }
        rc = build_cmd_buff(io_buff, &cmd);
        if (rc == WARN_NO_CMDS) {
            send_message_string(cli_sock, CMD_WARN_NO_CMD);
            send_message_eof(cli_sock);
            free_cmd_buff(&cmd);
            continue;
        } else if (rc < 0) {
            send_message_string(cli_sock, "error: command parsing failure\n");
            send_message_eof(cli_sock);
            free_cmd_buff(&cmd);
            continue;
        }
        
        // First, check for built-in commands using the new built-in handler.
        Built_In_Cmds bic = rsh_exec_built_in_cmd(cli_sock, &cmd);
        if (bic != BI_NOT_BI) {
            if (bic == BI_CMD_EXIT)
                break;
            else if (bic == BI_CMD_STOP_SVR) {
                server_stop = 1;
                break;
            }
            free_cmd_buff(&cmd);
            continue;
        }
        
        // If not built-in, execute the command (or pipeline).
        if (strchr(io_buff, PIPE_CHAR) != NULL) {
            command_list_t cmd_list;
            memset(&cmd_list, 0, sizeof(cmd_list));
            rc = build_cmd_list(io_buff, &cmd_list);
            if (rc != OK) {
                if (rc == ERR_TOO_MANY_COMMANDS)
                    send_message_string(cli_sock, "error: piping limited to 8 commands\n");  
                else
                    send_message_string(cli_sock, "error: command parsing error\n");
                send_message_eof(cli_sock);  
                free_cmd_list(&cmd_list);
                free_cmd_buff(&cmd);
                continue;
            }
            rc = rsh_execute_pipeline(cli_sock, &cmd_list);
            free_cmd_list(&cmd_list);
        } else {
            rc = remote_exec_cmd_with_redirection(cli_sock, &cmd);
        }
        free_cmd_buff(&cmd);
    }
    free(io_buff);
    return rc;
}


/*--------------------------------------------------------------
   client_handler (thread function)
--------------------------------------------------------------*/
void *client_handler(void *arg) {
    int cli_sock = *(int *)arg;
    free(arg);
    int rc = exec_client_requests(cli_sock);
    close(cli_sock);
    pthread_exit((void *)(intptr_t)rc);
}

/*--------------------------------------------------------------
   process_cli_requests(svr_socket)
--------------------------------------------------------------*/
int process_cli_requests(int svr_socket) {
    int rc = OK;
    while (!server_stop) {
        fd_set readfds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_SET(svr_socket, &readfds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        int sel = select(svr_socket + 1, &readfds, NULL, NULL, &tv);
        if (sel < 0) {
            perror("select");
            break;
        } else if (sel == 0) {
            continue; // timeout: loop and check server_stop again
        }
        int *cli_socket_ptr = malloc(sizeof(int));
        if (cli_socket_ptr == NULL) {
            fprintf(stderr, "Memory allocation error\n");
            rc = ERR_RDSH_SERVER;
            break;
        }
        *cli_socket_ptr = accept(svr_socket, NULL, NULL);
        if (*cli_socket_ptr < 0) {
            perror("accept");
            free(cli_socket_ptr);
            rc = ERR_RDSH_COMMUNICATION;
            break;
        }
        pthread_t tid;
        int err = pthread_create(&tid, NULL, client_handler, cli_socket_ptr);
        if (err != 0) {
            fprintf(stderr, "pthread_create error: %s\n", strerror(err));
            free(cli_socket_ptr);
            rc = ERR_RDSH_SERVER;
            break;
        }
        pthread_detach(tid);
    }
    return rc;
}

/*--------------------------------------------------------------
   start_server(ifaces, port, is_threaded)
--------------------------------------------------------------*/
int start_server(char *ifaces, int port, int is_threaded) {
    (void)is_threaded; // not used
    int svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0)
        return svr_socket;
    
    int rc = process_cli_requests(svr_socket);
    stop_server(svr_socket);
    return rc;
}
