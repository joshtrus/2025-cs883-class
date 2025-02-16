#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
static char* trim(char *s)
{
    while (isspace((unsigned char)*s)) {
        s++;
    }

    if (*s == 0) {
        return s;
    }

    char *end = s + strlen(s) - 1;

    while (end > s && isspace((unsigned char)*end)) {
        end--;
    }

    end[1] = '\0';
    return s;
}

int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    char *line = trim(cmd_line);

    if (strlen(line) == 0)
    {
        return WARN_NO_CMDS;
    }

    char *commands[CMD_MAX];
    int count = 0;

    char *saveptr;
    char *token = strtok_r(line, PIPE_STRING, &saveptr);

    while (token != NULL)
    {
        token = trim(token);

        if (count >= CMD_MAX)
        {
            return ERR_TOO_MANY_COMMANDS;
        }

        commands[count++] = token;

        token = strtok_r(NULL, PIPE_STRING, &saveptr);
    }

    clist->num = count;

    for (int i = 0; i < count; i++)
    {
        char *cmd_str = commands[i];

        if (strlen(cmd_str) == 0)
        {
            clist->commands[i].exe[0] = '\0';
            clist->commands[i].args[0] = '\0';
            continue;
        }

        char *saveptr2;
        char *exe_token = strtok_r(cmd_str, " \t", &saveptr2);
        if (exe_token == NULL)
        {
            clist->commands[i].exe[0] = '\0';
            clist->commands[i].args[0] = '\0';
            continue;
        }

        if (strlen(exe_token) >= EXE_MAX)
        {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        strcpy(clist->commands[i].exe, exe_token);

        char *arg_ptr = clist->commands[i].args;
        arg_ptr[0] = '\0';  

        char *arg_token = strtok_r(NULL, "", &saveptr2);

        if (arg_token != NULL)
        {
            arg_token = trim(arg_token);

            if (strlen(arg_token) >= ARG_MAX)
            {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }

            strcpy(arg_ptr, arg_token);
        }
    }

    return OK;
}
