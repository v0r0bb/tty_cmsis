#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "uart.h"

struct uart_state {
    struct uart_cfg cfg;
    USART_TypeDef *USARTx;
    uint8_t rx_buffer[UART_BUFFER_SIZE], tx_buffer[UART_BUFFER_SIZE];
    uint16_t rx_head, rx_tail;
    uint16_t tx_head, tx_tail;
    bool initialized;
};
static struct uart_state states[INSTANSE_NUM];

static void uart_rcc_enable(enum uart_instanse_id instanse_id);
static void uart_gpio_init(enum uart_instanse_id instanse_id);
static void uart_brr_init(enum uart_instanse_id instanse_id); 
static void uart_interrupt_handler(enum uart_instanse_id instanse_id);

int32_t uart_init(struct uart_cfg *cfg, enum uart_instanse_id instanse_id)
{
    if (instanse_id >= INSTANSE_NUM)
        return TTY_ERROR_BAD_ID;

    struct uart_state *state = &states[instanse_id];
    memset(state, 0, sizeof(*state));
    state->cfg = *cfg;

    switch (instanse_id) {
        case UART1:    state->USARTx = USART1; break;
        case UART2:    state->USARTx = USART2; break;
        case UART3:    state->USARTx = USART3; break;
        default:       break;
    }
    uart_rcc_enable(instanse_id);
    uart_gpio_init(instanse_id);

    state->USARTx->CR1 = 0UL;
    state->USARTx->CR2 = 0UL;
    state->USARTx->CR3 = 0UL;

    state->USARTx->CR1 |= (cfg->data_width == 9 ? 1 : 0) << USART_CR1_M_Pos;

    switch (cfg->stop_bits) {
        case 1:    state->USARTx->CR2 &= ~USART_CR2_STOP; break;
        case 2:    state->USARTx->CR2 |= USART_CR2_STOP_1; break;
        default:   state->USARTx->CR2 &= ~USART_CR2_STOP; break;
    }

    switch (cfg->parity_sel) {
        case 'E': 
            state->USARTx->CR1 |= USART_CR1_PCE;
            state->USARTx->CR1 &= ~USART_CR1_PS; 
            break;
        case 'O': 
            state->USARTx->CR1 |= USART_CR1_PCE;
            state->USARTx->CR1 |=  USART_CR1_PS;
            break;
        case 'N':
        default:
            break;
    }
            

    switch (cfg->over_sampling) {
        case 0:    state->USARTx->CR1 &= ~USART_CR1_OVER8; break;
        case 1:    state->USARTx->CR1 |= USART_CR1_OVER8; break;
        default:   state->USARTx->CR1 &= ~USART_CR1_OVER8; break;
    }
    uart_brr_init(instanse_id);

    state->initialized = true;
    return 1;
}

void uart_deinit(enum uart_instanse_id instanse_id)
{
    if (instanse_id >= INSTANSE_NUM)
        return;
    struct uart_state *state = &states[instanse_id];
    
    state->USARTx->CR1 = 0UL;
    state->USARTx->CR2 = 0UL;
    state->USARTx->CR3 = 0UL;

    state->rx_head = state->rx_tail = 0;
    state->tx_head = state->tx_tail = 0;
    memset(&state->rx_buffer, 0, UART_BUFFER_SIZE);
    memset(&state->tx_buffer, 0, UART_BUFFER_SIZE);

    switch (instanse_id) {
        case UART1:    NVIC_DisableIRQ(USART1_IRQn); break;
        case UART2:    NVIC_DisableIRQ(USART2_IRQn); break;
        case UART3:    NVIC_DisableIRQ(USART3_IRQn); break;
    }

    return;
}

int32_t uart_start(enum uart_instanse_id instanse_id)
{
    struct uart_state *state = &states[instanse_id];
    if (state->initialized == false)
        return TTY_ERROR_BAD_ID;

    state->USARTx->CR1 |= (USART_CR1_TE | USART_CR1_RE | USART_CR1_UE);  
    state->USARTx->CR1 |= (USART_CR1_TXEIE | USART_CR1_RXNEIE); 
    state->USARTx->CR3 |= USART_CR3_EIE;  
    
    switch (instanse_id) {
        case UART1:    NVIC_EnableIRQ(USART1_IRQn); break;
        case UART2:    NVIC_EnableIRQ(USART2_IRQn); break;
        case UART3:    NVIC_EnableIRQ(USART3_IRQn); break;
    }

    return 1;
}

static void uart_rcc_enable(enum uart_instanse_id instanse_id)
{
    switch(instanse_id) {
        case UART1:
            RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
            RCC->AHBENR  |= RCC_AHBENR_GPIOAEN; 
            break;
        case UART2:
            RCC->APB1ENR |= RCC_APB1ENR_USART2EN;  
            RCC->AHBENR  |= RCC_AHBENR_GPIOAEN; 
            break;
        case UART3:
            RCC->APB1ENR |= RCC_APB1ENR_USART3EN;  
            RCC->AHBENR  |= RCC_AHBENR_GPIOBEN; 
            break;
        default:
            break;
    }
}

static void uart_gpio_init(enum uart_instanse_id instanse_id)
{
        switch(instanse_id) {
        case UART1:
        case UART2:
        case UART3:
                GPIOA->MODER &= ~(GPIO_MODER_MODER2 | GPIO_MODER_MODER3);
                GPIOA->MODER |= (2 << GPIO_MODER_MODER2_Pos) | (2 << GPIO_MODER_MODER3_Pos);

                GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL2;
                GPIOA->AFR[0] |= (7 << GPIO_AFRL_AFSEL2_Pos);

                GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL3;
                GPIOA->AFR[0] |= (7 << GPIO_AFRL_AFSEL3_Pos);

                GPIOA->OSPEEDR |= (3 << GPIO_OSPEEDR_OSPEEDR2_Pos) | (3 << GPIO_OSPEEDR_OSPEEDR3_Pos);

                GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR2 | GPIO_PUPDR_PUPDR3);
                GPIOA->OTYPER &= ~(GPIO_OTYPER_OT_2 | GPIO_OTYPER_OT_3);
            break;
        default:
            break;
    }
}

static void uart_brr_init(enum uart_instanse_id instanse_id) 
{
    uint32_t sysclk_source = RCC->CFGR & RCC_CFGR_SWS;
    uint32_t sysclk_freq;

    uint32_t APB1_freq, APB2_freq;
    uint32_t APB1_prescaler = RCC->CFGR & RCC_CFGR_PPRE1;
    uint32_t APB2_prescaler = RCC->CFGR & RCC_CFGR_PPRE2;

    switch(sysclk_source) {
        case 1:    sysclk_freq = HSI_FREQ; break;
        case 2:    sysclk_freq = HSE_FREQ; break;
        default:   sysclk_freq = HSI_FREQ; break;  
    }

    switch(APB1_prescaler) {
        case 0:    APB1_freq = sysclk_freq; break;
        case 4:    APB1_freq = sysclk_freq / 2; break;
        case 5:    APB1_freq = sysclk_freq / 4; break;
        case 6:    APB1_freq = sysclk_freq / 8; break;
        case 7:    APB1_freq = sysclk_freq / 16; break;
        default:   APB1_freq = sysclk_freq; break;
    }

    switch(APB2_prescaler) {
        case 0:    APB2_freq = sysclk_freq; break;
        case 4:    APB2_freq = sysclk_freq / 2; break;
        case 5:    APB2_freq = sysclk_freq / 4; break;
        case 6:    APB2_freq = sysclk_freq / 8; break;
        case 7:    APB2_freq = sysclk_freq / 16; break;
        default:   APB2_freq = sysclk_freq; break;
    }
    
    struct uart_state *state = &states[instanse_id];
    uint32_t over8 = state->cfg.over_sampling;
    uint32_t baud_rate = state->cfg.baud_rate;
    uint64_t usart_div;
    
    switch (instanse_id) {
        case UART1:    
            usart_div = ((uint64_t)APB2_freq * 10000) / (8 * (2 - over8) * baud_rate); 
            break;
        case UART2:
        case UART3:
        default:
            usart_div = ((uint64_t)APB1_freq * 10000) / (8 * (2 - over8) * baud_rate); 
            break;
    }

    uint32_t div_mantissa = usart_div / 10000;
    uint32_t fraction = usart_div % 10000;
    uint32_t div_fraction = (fraction * 16) / 10000;

    state->USARTx->BRR = (div_mantissa << 4) | div_fraction;
}

void USART1_IRQHandler(void)
{
    uart_interrupt_handler(UART1);
}

void USART2_IRQHandler(void)
{
    uart_interrupt_handler(UART2);
}

void USART3_IRQHandler(void)
{
    uart_interrupt_handler(UART3);
}

static void uart_interrupt_handler(enum uart_instanse_id instanse_id)
{
    struct uart_state *state = &states[instanse_id];
    uint32_t uart_status = state->USARTx->SR;

    if (uart_status & USART_SR_RXNE) {
        uint8_t data;
        data = state->USARTx->DR;
        uint16_t next_rx_tail = (state->rx_tail + 1) % UART_BUFFER_SIZE;
        if (next_rx_tail != state->rx_tail) {
            state->rx_buffer[state->rx_tail] = data;
            state->rx_tail = next_rx_tail;
        }
        // overrun error?
        
    }
    if ((uart_status & USART_SR_TXE) && (state->USARTx->CR1 & USART_CR1_TXEIE)) {
        if (state->tx_head != state->tx_tail) {
            state->USARTx->DR = state->tx_buffer[state->tx_head];
            state->tx_head = (state->tx_head + 1) % UART_BUFFER_SIZE;
        }
        else
            state->USARTx->CR1 &= ~USART_CR1_TXEIE;
    }
    // usart error interrupts?
}

int32_t uart_putc(enum uart_instanse_id instanse_id, uint8_t data)
{
    if (instanse_id >= INSTANSE_NUM)
        return TTY_ERROR_BAD_ID;
    struct uart_state *state = &states[instanse_id];

    uint16_t next_tx_tail = (state->tx_tail + 1) % UART_BUFFER_SIZE;
    if (next_tx_tail == state->tx_head)
        return false;
    state->tx_buffer[state->tx_tail] = data;
    state->tx_tail = next_tx_tail;
    state->USARTx->CR1 |= USART_CR1_TXEIE;

    return 1;
}

int32_t uart_getc(enum uart_instanse_id instanse_id, uint8_t *data)
{
    if (instanse_id >= INSTANSE_NUM)
        return TTY_ERROR_BAD_ID;
    struct uart_state *state = &states[instanse_id];

    if (state->rx_head == state->rx_tail)
        return 0;
    *data = state->rx_buffer[state->rx_head];
    state->rx_head = (state->rx_head + 1) % UART_BUFFER_SIZE;

    return 1;
}

int32_t uart_puts(enum uart_instanse_id instanse_id, const uint8_t *data_buffer)
{
    if (instanse_id >= INSTANSE_NUM)
        return TTY_ERROR_BAD_ID;

    struct uart_state *state = &states[instanse_id];
    if (!state->initialized)
        return TTY_ERROR_STATE;
    
    int32_t cnt = 0;
    while (*data_buffer && uart_putc(instanse_id, *data_buffer)) {
        data_buffer++;
        cnt++;
    }
    return cnt;
}

int _write(int file, char* ptr, int len)
{
    for (int i = 0; i < len; i++) {
        char data = *ptr++;
        uart_putc(UART2, data);
        if (data == '\n')
            uart_putc(UART2, '\r');
    }

    return len;
}
