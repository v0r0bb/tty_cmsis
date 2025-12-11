#ifndef CMD_H
#define CMD_H

#include <stdint.h>
#include "error.h"

#define CMD_MAX_CLI 10

typedef int32_t (*cmd_func)(int32_t argc, const char **argv);

struct cmd_cmd_info {
    const char * const name;
    const cmd_func func;
    const char * const help;
};

struct cmd_client_info {
    const char * const name;
    int32_t num_cmds;
    const struct cmd_cmd_info * const cmds;
};

int32_t cmd_register(const struct cmd_client_info* client_info);
int32_t cmd_execute(const uint8_t *buffer);

#endif