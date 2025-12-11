#ifndef SHELL_H
#define SHELL_H

#include <stdbool.h>
#include "uart.h"


#define PROMPT "# "
#define CMD_BUFFER_SIZE 100

void shell_init(enum uart_instanse_id instanse_id);
void shell_run(void);


#endif