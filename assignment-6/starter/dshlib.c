#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include "dshlib.h"

/*--------------------------------------------------------------
  Allocation and Cleanup Functions
--------------------------------------------------------------*/
int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (!cmd_buff->_cmd_buffer)
        return ERR_MEMORY;
    for (int i = 0; i < SH_CMD_MAX; i++)
        cmd_buff->_cmd_buffer[i] = '\0';
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++)
        cmd_buff->argv[i] = NULL;
    cmd_buff->input_file = NULL;
    cmd_buff->output_file = NULL;
    cmd_buff->append_mode = false;
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }
    cmd_buff->argc = 0;
    return OK;
}

int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer) {
        for (int i = 0; i < SH_CMD_MAX; i++)
            cmd_buff->_cmd_buffer[i] = '\0';
    }
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++)
        cmd_buff->argv[i] = NULL;
    cmd_buff->input_file = NULL;
    cmd_buff->output_file = NULL;
    cmd_buff->append_mode = false;
    return OK;
}

/*--------------------------------------------------------------
  Command Parsing Functions
--------------------------------------------------------------*/
/*
 * build_cmd_buff: Parses a single command string into a cmd_buff_t.
 * It trims whitespace, tokenizes the command (respecting quoted strings),
 * and then scans for I/O redirection tokens (<, >, >>).
 */
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd) {
    strncpy(cmd->_cmd_buffer, cmd_line, SH_CMD_MAX - 1);
    
    // Reset redirection fields.
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->append_mode = false;
    
    char *start = cmd->_cmd_buffer;
    while (isspace((unsigned char)*start))
        start++;
    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
    if (*start == '\0')
        return WARN_NO_CMDS;
    
    bool in_quotes = false;
    char *dst = start;
    char *arg_start = NULL;
    cmd->argc = 0;
    while (*start) {
        if (*start == '"') {
            in_quotes = !in_quotes;
            start++;
            continue;
        }
        if (!in_quotes && isspace((unsigned char)*start)) {
            *dst++ = '\0';
            if (arg_start) {
                cmd->argv[cmd->argc++] = arg_start;
                arg_start = NULL;
            }
            while (isspace((unsigned char)*(start + 1)))
                start++;
        } else {
            if (!arg_start)
                arg_start = dst;
            *dst++ = *start;
        }
        start++;
    }
    if (arg_start) {
        *dst = '\0';
        cmd->argv[cmd->argc++] = arg_start;
    }
    cmd->argv[cmd->argc] = NULL;
    
    // Process I/O redirection tokens.
    int i = 0;
    while (i < cmd->argc) {
        if (strcmp(cmd->argv[i], "<") == 0) {
            if (i + 1 < cmd->argc)
                cmd->input_file = cmd->argv[i + 1];
            for (int j = i; j < cmd->argc - 2; j++)
                cmd->argv[j] = cmd->argv[j + 2];
            cmd->argc -= 2;
            continue;
        } else if (strcmp(cmd->argv[i], ">") == 0) {
            if (i + 1 < cmd->argc) {
                cmd->output_file = cmd->argv[i + 1];
                cmd->append_mode = false;
            }
            for (int j = i; j < cmd->argc - 2; j++)
                cmd->argv[j] = cmd->argv[j + 2];
            cmd->argc -= 2;
            continue;
        } else if (strcmp(cmd->argv[i], ">>") == 0) {
            if (i + 1 < cmd->argc) {
                cmd->output_file = cmd->argv[i + 1];
                cmd->append_mode = true;
            }
            for (int j = i; j < cmd->argc - 2; j++)
                cmd->argv[j] = cmd->argv[j + 2];
            cmd->argc -= 2;
            continue;
        }
        i++;
    }
    cmd->argv[cmd->argc] = NULL;
    return OK;
}

/*
 * build_cmd_list: Splits a full command line (which may contain pipes) into a command_list_t.
 */
int build_cmd_list(char *cmd_line, command_list_t *clist) {
    char *token;
    char *rest = cmd_line;
    int count = 0;
    while ((token = strtok_r(rest, PIPE_STRING, &rest)) != NULL) {
         while (isspace((unsigned char)*token))
             token++;
         char *end = token + strlen(token) - 1;
         while (end > token && isspace((unsigned char)*end)) {
             *end = '\0';
             end--;
         }
         if (*token == '\0')
             continue;
         if (count >= CMD_MAX)
             return ERR_TOO_MANY_COMMANDS;
         if (alloc_cmd_buff(&clist->commands[count]) != OK)
             return ERR_MEMORY;
         int rc = build_cmd_buff(token, &clist->commands[count]);
         if (rc < 0)
             return rc;
         count++;
    }
    clist->num = count;
    return OK;
}

int free_cmd_list(command_list_t *cmd_lst) {
    for (int i = 0; i < cmd_lst->num; i++) {
         free_cmd_buff(&cmd_lst->commands[i]);
    }
    return OK;
}

/*--------------------------------------------------------------
  Pipeline Execution
--------------------------------------------------------------*/
/*
 * execute_pipeline: Executes a pipeline of commands.
 * For each command in the list, forks a child process, sets up pipes,
 * handles I/O redirection from the command buffer, and calls execvp().
 */
int execute_pipeline(command_list_t *clist) {
    int num_cmds = clist->num;
    if (num_cmds == 0)
         return WARN_NO_CMDS;
    
    int pipefd[2];
    int prev_fd = -1;  // read end from the previous pipe
    pid_t pids[CMD_MAX];
    
    for (int i = 0; i < num_cmds; i++) {
         if (i < num_cmds - 1) {
             if (pipe(pipefd) == -1) {
                 perror("pipe");
                 return ERR_MEMORY;
             }
         }
         pid_t pid = fork();
         if (pid < 0) {
             perror("fork");
             return ERR_EXEC_CMD;
         } else if (pid == 0) {
             // Child process:
             if (i > 0) {
                 if (dup2(prev_fd, STDIN_FILENO) == -1) {
                     perror("dup2 (stdin)");
                     exit(errno);
                 }
             }
             if (i < num_cmds - 1) {
                 if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
                     perror("dup2 (stdout)");
                     exit(errno);
                 }
             }
             // Handle redirection for the current command.
             cmd_buff_t *curr = &clist->commands[i];
             if (curr->input_file != NULL) {
                 int fd_in = open(curr->input_file, O_RDONLY);
                 if (fd_in < 0) {
                     perror("open input");
                     exit(errno);
                 }
                 dup2(fd_in, STDIN_FILENO);
                 close(fd_in);
             }
             if (curr->output_file != NULL) {
                 int flags = O_WRONLY | O_CREAT;
                 flags |= curr->append_mode ? O_APPEND : O_TRUNC;
                 int fd_out = open(curr->output_file, flags, 0644);
                 if (fd_out < 0) {
                     perror("open output");
                     exit(errno);
                 }
                 dup2(fd_out, STDOUT_FILENO);
                 close(fd_out);
             }
             if (prev_fd != -1)
                 close(prev_fd);
             if (i < num_cmds - 1) {
                 close(pipefd[0]);
                 close(pipefd[1]);
             }
             execvp(curr->argv[0], curr->argv);
             perror("execvp");
             exit(errno);
         } else {
             pids[i] = pid;
             if (prev_fd != -1)
                 close(prev_fd);
             if (i < num_cmds - 1) {
                 prev_fd = pipefd[0];
                 close(pipefd[1]);
             }
         }
    }
    int status = 0;
    for (int i = 0; i < num_cmds; i++) {
         waitpid(pids[i], &status, 0);
    }
    return OK;
}

/*--------------------------------------------------------------
  Built-In Command Handling
--------------------------------------------------------------*/
Built_In_Cmds match_command(const char *input) {
    if (!input || !*input)
         return BI_NOT_BI;
    if (strcmp(input, EXIT_CMD) == 0)
         return BI_CMD_EXIT;
    if (strcmp(input, "cd") == 0)
         return BI_CMD_CD;
    if (strcmp(input, "dragon") == 0)
         return BI_CMD_DRAGON;
    if (strcmp(input, "rc") == 0)
         return BI_RC;
    return BI_NOT_BI;
}

int last_rc = 0;

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    Built_In_Cmds bic = match_command(cmd->argv[0]);
    switch (bic) {
         case BI_CMD_EXIT:
              return BI_CMD_EXIT;
         case BI_CMD_CD:
              if (cmd->argc > 1) {
                   if (chdir(cmd->argv[1]) != 0)
                        perror("cd");
              }
              return BI_EXECUTED;
         case BI_RC:
              printf("%d\n", last_rc);
              return BI_EXECUTED;
         default:
              return BI_NOT_BI;
    }
}

/*--------------------------------------------------------------
  Single Command Execution
--------------------------------------------------------------*/
int exec_cmd(cmd_buff_t *cmd) {
    pid_t pid = fork();
    if (pid < 0) {
         perror("fork");
         return ERR_EXEC_CMD;
    } else if (pid == 0) {
         if (cmd->input_file != NULL) {
             int fd_in = open(cmd->input_file, O_RDONLY);
             if (fd_in < 0) {
                 perror("open input file");
                 exit(errno);
             }
             dup2(fd_in, STDIN_FILENO);
             close(fd_in);
         }
         if (cmd->output_file != NULL) {
             int flags = O_WRONLY | O_CREAT;
             flags |= cmd->append_mode ? O_APPEND : O_TRUNC;
             int fd_out = open(cmd->output_file, flags, 0644);
             if (fd_out < 0) {
                 perror("open output file");
                 exit(errno);
             }
             dup2(fd_out, STDOUT_FILENO);
             close(fd_out);
         }
         execvp(cmd->argv[0], cmd->argv);
         perror("execvp");
         exit(errno);
    } else {
         int status = 0;
         waitpid(pid, &status, 0);
         if (WIFEXITED(status)) {
             last_rc = WEXITSTATUS(status);
             if (last_rc == ENOENT)
                 fprintf(stderr, "Command not found in PATH\n");
             else if (last_rc == EACCES)
                 fprintf(stderr, "Permission denied\n");
             else if (last_rc != 0)
                 fprintf(stderr, CMD_ERR_EXECUTE);
             return last_rc;
         }
    }
    return OK;
}

/*--------------------------------------------------------------
  Main Local Command Loop
--------------------------------------------------------------*/
int exec_local_cmd_loop() {
    char line[SH_CMD_MAX];
    cmd_buff_t cmd;
    command_list_t clist;
    int rc;
    
    if (alloc_cmd_buff(&cmd) != OK) {
         fprintf(stderr, "Error: cannot allocate memory for cmd buffer.\n");
         return ERR_MEMORY;
    }
    
    while (1) {
         printf("%s", SH_PROMPT);
         if (!fgets(line, SH_CMD_MAX, stdin)) {
              printf("\n");
              break;
         }
         line[strcspn(line, "\n")] = '\0';
         
         if (strcmp(line, EXIT_CMD) == 0) {
              printf("exiting...\n");
              break;
         }
         
         if (strchr(line, PIPE_CHAR) != NULL) {
              memset(&clist, 0, sizeof(clist));
              rc = build_cmd_list(line, &clist);
              if (rc != OK) {
                   if (rc == ERR_TOO_MANY_COMMANDS)
                        printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
                   continue;
              }
              rc = execute_pipeline(&clist);
              free_cmd_list(&clist);
         } else {
              clear_cmd_buff(&cmd);
              rc = build_cmd_buff(line, &cmd);
              if (rc == WARN_NO_CMDS) {
                   printf(CMD_WARN_NO_CMD);
                   continue;
              } else if (rc < 0) {
                   continue;
              }
              Built_In_Cmds bic = exec_built_in_cmd(&cmd);
              if (bic == BI_CMD_EXIT)
                   break;
              else if (bic == BI_EXECUTED)
                   continue;
              int exec_rc = exec_cmd(&cmd);
              (void) exec_rc;
         }
    }
    free_cmd_buff(&cmd);
    return OK;
}
