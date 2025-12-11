#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "shell.h"
#include "uart.h"
#include "cmd.h"

struct shell_state {
    enum uart_instanse_id instanse_id;
    uint8_t cmd_buffer[CMD_BUFFER_SIZE];
    uint16_t cmd_buffer_idx;
    bool first_run;
};

static struct shell_state state;

void shell_init(enum uart_instanse_id instanse_id)
{
    memset(&state, 0, sizeof(state));
    state.instanse_id = instanse_id;

    /* instead of flush() */
    setvbuf(stdout, NULL, _IONBF, 0);
    return;
}

void shell_run(void)
{
    uint8_t data;
    if (!state.first_run) {
        printf("%s", PROMPT);
        state.first_run = true;
    }

    while (uart_getc(state.instanse_id, &data)) {
        if (data == '\n' || data == '\r') {
            state.cmd_buffer[state.cmd_buffer_idx] = '\0';
            printf("\n");
            cmd_execute(state.cmd_buffer);
            state.cmd_buffer_idx = 0;
            state.cmd_buffer[state.cmd_buffer_idx] = '\0';
            printf("%s", PROMPT);
            continue;
        }
        if (data == '\b' || data == '\x7f') {
            if (state.cmd_buffer_idx > 0) {
                printf("\b \b");
                state.cmd_buffer_idx--;
            }
            continue;
        }
        if (isprint(data)) {
            if (state.cmd_buffer_idx < (CMD_BUFFER_SIZE - 1)) {
                state.cmd_buffer[state.cmd_buffer_idx++] = data;
                printf("%c", data);
            }
            else 
                printf("\a");
            continue;
        }
    }
}