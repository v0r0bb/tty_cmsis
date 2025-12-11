#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "cmd.h"
#include "shell.h"

#define CMD_MAX_TOKENS 10

static const struct cmd_client_info* clients_info[CMD_MAX_CLI];

int32_t cmd_register(const struct cmd_client_info* client_info)
{
    for (uint32_t i = 0; i < CMD_MAX_CLI; i++) {
        if (clients_info[i] == NULL ||
            !strcmp(clients_info[i]->name, client_info->name)) {
                clients_info[i] = client_info;
                return 0;
            }
    }
    return TTY_ERROR_ARG;
}

int32_t cmd_execute(const uint8_t* buffer)
{
    uint32_t num_tokens = 0;
    const uint8_t *tokens[CMD_MAX_TOKENS];
    uint8_t local_buffer[CMD_BUFFER_SIZE];
    uint8_t *pbuf;
    uint32_t idx_cli, idx_cmd;
    const struct cmd_client_info *client_info;
    const struct cmd_cmd_info *cmd_info;

    memcpy(local_buffer, buffer, CMD_BUFFER_SIZE);
    pbuf = local_buffer;

    while(1) {
        while (*pbuf && isspace(*pbuf))
            pbuf++;

        if (*pbuf == '\0')
            break;
        else {
            if (num_tokens >= CMD_MAX_TOKENS) {
                printf("Too many tokens\n");
                return TTY_ERROR_ARG;
            }
            tokens[num_tokens++] = pbuf;
            while(*pbuf && !isspace(*pbuf))
                pbuf++;
            
            if (*pbuf)
                *pbuf++ = '\0';
            else 
                break;
        }
    }

    if (num_tokens == 0)
        return 0;

    if (!strcmp("help", (char *)tokens[0])) {
        for (idx_cli = 0; idx_cli < CMD_MAX_CLI && clients_info[idx_cli] != NULL; idx_cli++) {
            printf("Usage: <client> COMMAND\n");
            client_info = clients_info[idx_cli];
            if (client_info->num_cmds == 0)
                continue;
            printf("%s commands: \n", client_info->name);
            for (idx_cmd = 0; idx_cmd < client_info->num_cmds; idx_cmd++) {
                cmd_info = &client_info->cmds[idx_cmd];
                printf("\t%s\t%s\n", cmd_info->name, cmd_info->help);
            }
        }
        return 0; 
    }

    for (idx_cli = 0; idx_cli < CMD_MAX_CLI && clients_info[idx_cli] != NULL; idx_cli++) {
        client_info = clients_info[idx_cli];
        if (strcmp((char *)tokens[0], client_info->name)) 
            continue;

        if (num_tokens == 1 || !strcmp((char *)tokens[1], "help")) {
            for (idx_cmd = 0; idx_cmd < client_info->num_cmds; idx_cmd++) {
                cmd_info = &client_info->cmds[idx_cmd];
                printf("%s\t%s\n", cmd_info->name, cmd_info->help);
            }
            return 0;
        }

        for (idx_cmd = 0; idx_cmd < client_info->num_cmds; idx_cmd++) {
            cmd_info = &client_info->cmds[idx_cmd];
            if (!strcmp((char *)tokens[1], cmd_info->name)) {

                if (!strcmp((char *)tokens[2], "help")) {
                    printf("%s\n", cmd_info->help);
                    return 0;
                }
                const char *argv[CMD_MAX_TOKENS];
                for (uint8_t i = 0; i < num_tokens; i++) 
                    argv[i] = (const char *)tokens[i];

                return cmd_info->func(num_tokens, argv);
            }
        }
        printf("No such command <%s>\n", tokens[1]);
        return TTY_ERROR_ARG;
    }
    printf("No such client <%s>\n", tokens[0]);
    return TTY_ERROR_ARG;
}