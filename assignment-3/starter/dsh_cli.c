#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dshlib.h"

static const char DRAGON_B64[] = "ICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgQCUlJSUgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAlJSUlJSUgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgJSUlJSUlICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAlICUlJSUlJSUgICAgICAgICAgIEAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgJSUlJSUlJSUlJSAgICAgICAgJSUlJSUlJSAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICUlJSUlJSUgICUlJSVAICAgICAgICAgJSUlJSUlJSUlJSUlQCAgICAlJSUlJSUgIEAlJSUlICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgJSUlJSUlJSUlJSUlICUlJSUlJSUlJSUlJSUlJSAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAlJSUlJSUlJSUlJSUlJSUlJSUlICAgICAlJSUgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlQCBAJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgICAlJSAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlQCUlJSUlJUAgICAgICAgICAgICAgIAogICAgICAlJSUlJSUlJUAgICAgICAgICAgICUlJSUlJSUlJSUlJSUlJSUgICAgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgJSUgICAgICAgICAgICAgICAgCiAgICAlJSUlJSUlJSUlJSUlICAgICAgICAgJSVAJSUlJSUlJSUlJSUlICAgICAgICAgICAlJSUlJSUlJSUlJSAlJSUlJSUlJSUlJSUgICAgICBAJSAgICAgICAgICAgICAgICAKICAlJSUlJSUlJSUlICAgJSUlICAgICAgICAlJSUlJSUlJSUlJSUlJSAgICAgICAgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgICAgICAgICAgICAgICAgICAgICAgIAogJSUlJSUlJSUlICAgICAgICUgICAgICAgICAlJSUlJSUlJSUlJSUlICAgICAgICAgICAgICUlJSUlJSUlJSUlJUAlJSUlJSUlJSUlJSAgICAgICAgICAgICAgICAgICAgICAgCiUlJSUlJSUlJUAgICAgICAgICAgICAgICAgJSAlJSUlJSUlJSUlJSUlICAgICAgICAgICAgQCUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUgICAgICAgICAgICAgICAgICAgICAKJSUlJSUlJSVAICAgICAgICAgICAgICAgICAlJUAlJSUlJSUlJSUlJSUgICAgICAgICAgICBAJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgICAgICAgICAgICAgICAgIAolJSUlJSUlQCAgICAgICAgICAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUgICAgICAgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgICAgICAgICAgICAgCiUlJSUlJSUlJSUgICAgICAgICAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUgICAgICAlJSUlICAKJSUlJSUlJSUlQCAgICAgICAgICAgICAgICAgICBAJSUlJSUlJSUlJSUlJSUgICAgICAgICAlJSUlJSUlJSUlJSVAICUlJSUgJSUlJSUlJSUlJSUlJSUlJSUgICAlJSUlJSUlJQolJSUlJSUlJSUlICAgICAgICAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlJSUgICAgICAgICUlJSUlJSUlJSUlJSUgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUgJSUlJSUlJSUlCiUlJSUlJSUlJUAlJUAgICAgICAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlJUAgICAgICAgJSUlJSUlJSUlJSUlJSUgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgJSUKICUlJSUlJSUlJSUgICAgICAgICAgICAgICAgICAlICUlJSUlJSUlJSUlJSUlQCAgICAgICAgJSUlJSUlJSUlJSUlJSUgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAlJQogICUlJSUlJSUlJSUlJSAgQCAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAlJSUgCiAgICUlJSUlJSUlJSUlJSUgJSUgICUgICVAICUlJSUlJSUlJSUlJSUlJSUlJSAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgICUlJSAKICAgICUlJSUlJSUlJSUlJSUlJSUlJSAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgICAgICBAJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgICAlJSUlJSUlIAogICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgICAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgICAgICAgJSUlICAgCiAgICAgIEAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgICAgICAgICAgICAgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUgICAgICAgICAgICAgICAKICAgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgICAgICAgICAgICAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlICAlJSUlJSUlICAgICAgICAgIAogICAgICAgICAgICUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgICAgICAgICAgICAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlICBAJSUlJSUlJSUlICAgICAgICAgCiAgICAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlJSUlJSUgICAgICAgICAgIEAlQCUgICAgICAgICAgICAgICAgICBAJSUlJSUlJSUlJSUlJSUlJSUlICAgJSUlICAgICAgICAKICAgICAgICAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlICAgICAgICAlJSUlJSUlJSUlICAgICAgICAgICAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUgICAgJSAgICAgICAgIAogICAgICAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlICAgICAgICAgICAgICAgICAgICAgICUlJSUlJSUlJSUlJSUlICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSUlJSAgJSUlJSAlJSUgICAgICAgICAgICAgICAgICAgICAgJSUlJSUlJSUlJSAgJSUlQCAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgJSUlJSUlJSUlJSUlJSUlJSUlJSAlJSUlJSUgJSUgICAgICAgICAgICAgICAgICAgICAgICAgICUlJSUlJSUlJSUlJSVAICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAlJSUlJSUlQCAgICAgICAK";


/*
 * Implement your main function by building a loop that prompts the
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.  Since we want fgets to also handle
 * end of file so we can run this headless for testing we need to check
 * the return code of fgets.  I have provided an example below of how
 * to do this assuming you are storing user input inside of the cmd_buff
 * variable.
 *
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 *
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 *
 *   Also, use the constants in the dshlib.h in this code.
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *
 *   Expected output:
 *
 *      CMD_OK_HEADER      if the command parses properly. You will
 *                         follow this by the command details
 *
 *      CMD_WARN_NO_CMD    if the user entered a blank command
 *      CMD_ERR_PIPE_LIMIT if the user entered too many commands using
 *                         the pipe feature, e.g., cmd1 | cmd2 | ... |
 *
 *  See the provided test cases for output expectations.
 */

static int b64_index(char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}


static int base64_decode(const char *in, unsigned char *out)
{
    int in_len = strlen(in);
    int out_len = 0;
    int pad = 0;

    for (int i = 0; i < in_len; i += 4)
    {
        int v = 0;
        for (int j = 0; j < 4; j++)
        {
            v <<= 6;
            if (in[i + j] == '=')
            {
                pad++;
            }
            else
            {
                int idx = b64_index(in[i + j]);
                if (idx < 0)
                {
                    return -1;
                }
                v |= (idx & 0x3F);
            }
        }
        out[out_len + 2] = (unsigned char)(v & 0xFF); 
        v >>= 8;
        out[out_len + 1] = (unsigned char)(v & 0xFF);
        v >>= 8;
        out[out_len + 0] = (unsigned char)(v & 0xFF);
        out_len += 3;
    }

    out_len -= pad;
    out[out_len] = '\0';  
    return out_len;
}

int main()
{

    char cmd_buff[SH_CMD_MAX];

    command_list_t clist;

    while (1)
    {
        printf("%s", SH_PROMPT);

        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL)
        {
            printf("\n");
            break;
        }

        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        if (strcmp(cmd_buff, EXIT_CMD) == 0)
        {
            return 0;
        }


        if (strcmp(cmd_buff, "dragon") == 0)
        {
            unsigned char dragon_ascii[8192]; 
            int decoded_len = base64_decode(DRAGON_B64, dragon_ascii);

            if (decoded_len > 0)
            {
                printf("%s\n", dragon_ascii);
            }
            else
            {
                fprintf(stderr, "Error decoding dragon data.\n");
            }
            continue;
        }

        memset(&clist, 0, sizeof(clist)); 
        int rc = build_cmd_list(cmd_buff, &clist);

        switch (rc)
        {
        case OK:
            if (clist.num == 0)
            {
                printf(CMD_WARN_NO_CMD);
            }
            else
            {
                printf(CMD_OK_HEADER, clist.num);
                for (int i = 0; i < clist.num; i++)
                {
                    printf("<%d> %s", i + 1, clist.commands[i].exe);

                    if (strlen(clist.commands[i].args) > 0)
                    {
                        printf(" [%s]", clist.commands[i].args);
                    }
                    printf("\n");
                }
            }
            break;

        case WARN_NO_CMDS:
            printf(CMD_WARN_NO_CMD);
            break;

        case ERR_TOO_MANY_COMMANDS:
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            break;

        case ERR_CMD_OR_ARGS_TOO_BIG:
            fprintf(stderr, "error: command or argument string too large\n");
            break;

        default:
            break;
        }
    }

    return 0; 
}
