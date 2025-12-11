#include <stdio.h>
#include "uart.h"
#include "shell.h"
#include "gpio.h"

void sysclk_config(void);

struct uart_cfg def_cfg = {
    .baud_rate      = 115200,
    .data_width     = 8,
    .stop_bits      = 1,
    .parity_sel     = 'N',
    .over_sampling  = 0
};

int main(void) 
{
    sysclk_config();
    uart_init(&def_cfg, UART2);
    uart_start(UART2);

    const char *hello_str = "\r\nHello from NUCLEO-L152RE!\r\n";
    printf("%s", hello_str);

    shell_init(UART2);
    gpio_init();

    while (1) {
        shell_run();
    }
}

void sysclk_config(void)
{
    RCC->CR |= RCC_CR_HSION;
    while(!(RCC->CR & RCC_CR_HSIRDY));

    FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ACC64 | FLASH_ACR_LATENCY;

    RCC->CR &= ~(RCC_CR_PLLON);

    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_HSI;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);

    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);

    /* AHB = APB1 = APB2 = SYSCLK (DIV)*/
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;     
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV1;   
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;  
}