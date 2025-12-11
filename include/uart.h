#ifndef USART_H
#define USART_H

#include "stm32l1xx.h"
#include <stdint.h>
#include <stdbool.h>

#include "error.h"

#define UART_BUFFER_SIZE    256
#define INSTANSE_NUM        3
#define HSI_FREQ            16000000UL
#define HSE_FREQ            16000000UL

enum uart_instanse_id {
    UART1 = 0,
    UART2,
    UART3
};

struct uart_cfg {
    uint32_t baud_rate;
    uint32_t data_width;
    uint32_t stop_bits;
    char     parity_sel;
    uint32_t over_sampling;
};

struct uart_state;


int32_t uart_init(struct uart_cfg *cfg, enum uart_instanse_id instanse_id);
void uart_deinit(enum uart_instanse_id instanse_id);
int32_t uart_start(enum uart_instanse_id instanse_id);

int32_t uart_putc(enum uart_instanse_id instanse_id, uint8_t data);
int32_t uart_getc(enum uart_instanse_id instanse_id, uint8_t *data);

int32_t uart_puts(enum uart_instanse_id instanse_id, const uint8_t *data_buffer);

#endif