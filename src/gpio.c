#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include "stm32l1xx.h"
#include "cmd.h"

#define IS_VALID_PIN(pin)                       \
    ((pin) && (pin)[0] == 'P' &&                \
    (pin)[1] >= 'A' && (pin)[1] <= 'H' &&       \
    (((pin)[2] >= '0' && (pin)[2] <= '9' &&     \
    (pin)[3] == '\0') || ((pin)[2] == '1' &&    \
    (pin)[3] >= '0' && (pin)[3] <= '5' &&       \
    (pin)[4] == '\0')))

#define GET_PORT(pin) ((pin)[1])

#define GET_PIN(pin)                            \
    (!(pin)[3] ? ((pin)[2] - '0') :             \
    ((pin)[2] - '0') * 10 + (pin[3] - '0'))     \

#define GET_GPIO(port)                          \
    ((port) == 'A' ? GPIOA :                    \ 
    (port) == 'B' ? GPIOB :                     \
    (port) == 'C' ? GPIOC :                     \
    (port) == 'D' ? GPIOD :                     \ 
    (port) == 'E' ? GPIOE :                     \
    (port) == 'F' ? GPIOF :                     \
    (port) == 'G' ? GPIOG :                     \
    (port) == 'H' ? GPIOH :                     \
    GPIOA)

/* cmd functions */
static int32_t gpio_set_cmd(int32_t argc, const char **argv);
static int32_t gpio_reset_cmd(int32_t argc, const char **argv);
static int32_t gpio_toggle_cmd(int32_t argc, const char **argv);
static int32_t gpio_read_cmd(int32_t argc, const char **argv);

static const struct cmd_cmd_info gpio_cmds[] = {
    {.name = "set", .func = gpio_set_cmd, .help = "Set pin value: gpio set <pin>"},
    {.name = "reset", .func = gpio_reset_cmd, .help = "Resetet pin value: gpio reset <pin>"},
    {.name = "toggle", .func = gpio_toggle_cmd, .help = "Toggle pin value: gpio toggle <pin>"},
    {.name = "read", .func = gpio_read_cmd, .help = "Read pin value: gpio read <pin>"}
};

static const struct  cmd_client_info gpio_client = {
    .name = "gpio",
    .num_cmds = sizeof(gpio_cmds) / sizeof(gpio_cmds[0]),
    .cmds = gpio_cmds
};

/* CMSIS functions */
void gpio_rcc_enable(const char port);

void gpio_init(void)
{
    cmd_register(&gpio_client);
}

/* gpio set PA5*/
static int32_t gpio_set_cmd(int32_t argc, const char **argv)
{
    int32_t num;
    char port;
    GPIO_TypeDef *GPIOx;

    if (argc != 3 || !IS_VALID_PIN(argv[2])) {
        printf("Error: invalid args\n");
        return TTY_ERROR_ARG;
    }

    // printf("%d\n", IS_VALID_PIN(argv[2]));
    port = GET_PORT(argv[2]);
    num = GET_PIN(argv[2]);
    // printf("%ld, %c\n", num, port); 

    gpio_rcc_enable(port);
    GPIOx = GET_GPIO(port);

    GPIOx->MODER |= 1UL << (num * 2);
    GPIOx->BSRR |= 1UL << num;

    return 0;
}

static int32_t gpio_reset_cmd(int32_t argc, const char **argv)
{
    int32_t num;
    char port;
    GPIO_TypeDef *GPIOx;

    if (argc != 3 || !IS_VALID_PIN(argv[2])) {
        printf("Error: invalid args\n");
        return TTY_ERROR_ARG;
    }

    port = GET_PORT(argv[2]);
    num = GET_PIN(argv[2]);

    gpio_rcc_enable(port);
    GPIOx = GET_GPIO(port);

    GPIOx->MODER |= 1UL << (num * 2);
    GPIOx->BSRR |= 1UL << (num + 16);

    return 0;
}

static int32_t gpio_toggle_cmd(int32_t argc, const char **argv)
{
    int32_t num;
    char port;
    GPIO_TypeDef *GPIOx;

    if (argc != 3 || !IS_VALID_PIN(argv[2])) {
        printf("Error: invalid args\n");
        return TTY_ERROR_ARG;
    }

    port = GET_PORT(argv[2]);
    num = GET_PIN(argv[2]);

    gpio_rcc_enable(port);
    GPIOx = GET_GPIO(port);

    GPIOx->MODER |= 1UL << (num * 2);
    GPIOx->ODR ^= (1UL << num);
    
    return 0;
}

static int32_t gpio_read_cmd(int32_t argc, const char **argv)
{
    int32_t num;
    char port;
    GPIO_TypeDef *GPIOx;
    int32_t data;

    if (argc != 3 || !IS_VALID_PIN(argv[2])) {
        printf("Error: invalid args\n");
        return TTY_ERROR_ARG;
    }

    port = GET_PORT(argv[2]);
    num = GET_PIN(argv[2]);

    gpio_rcc_enable(port);
    GPIOx = GET_GPIO(port);

    GPIOx->MODER &= ~(3UL << (num * 2));

    data = (GPIOx->IDR & (1UL << num));
    printf("%ld\n", data);
    
    return 0;
}

void gpio_rcc_enable(const char port)
{
    switch (port) {
        case 'A': RCC->AHBENR |= RCC_AHBENR_GPIOAEN; break;
        case 'B': RCC->AHBENR |= RCC_AHBENR_GPIOBEN; break;
        case 'C': RCC->AHBENR |= RCC_AHBENR_GPIOCEN; break;
        case 'D': RCC->AHBENR |= RCC_AHBENR_GPIODEN; break;
        case 'E': RCC->AHBENR |= RCC_AHBENR_GPIOEEN; break;
        case 'F': RCC->AHBENR |= RCC_AHBENR_GPIOFEN; break;
        case 'G': RCC->AHBENR |= RCC_AHBENR_GPIOGEN; break;
        case 'H': RCC->AHBENR |= RCC_AHBENR_GPIOHEN; break;
    }
    return;
}
