/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-07-28     Tanek        the first version
 */
#include <rtthread.h>
#include "usart.h"

#include "peri_driver.h"

/* Forward declarations for power management */
extern void board_enter_sleep(void);
extern void board_enter_deep_sleep(void);
extern void board_enter_power_down(void);
extern void board_enter_deep_powerdown(void);

/* UART command buffer and processing */
#define UART_CMD_BUFFER_SIZE 64
static char uart_cmd_buffer[UART_CMD_BUFFER_SIZE];
static rt_size_t uart_cmd_pos = 0;

/**
 * @brief Process UART commands
 * @param cmd: Command string
 */
static void uart_process_command(char* cmd)
{
    /* Debug output */
    rt_kprintf("\r\n[CMD] %s\r\n", cmd);
    
    if (rt_strncmp(cmd, "sleep", 5) == 0) {
        rt_kprintf("Entering sleep mode...\r\n");
        rt_thread_mdelay(100);  /* Ensure message is sent before sleeping */
        board_enter_sleep();
    }
    else if (rt_strncmp(cmd, "deep_sleep", 10) == 0) {
        rt_kprintf("Entering deep sleep mode...\r\n");
        rt_thread_mdelay(100);
        board_enter_deep_sleep();
    }
    else if (rt_strncmp(cmd, "power_down", 10) == 0) {
        rt_kprintf("Entering power down mode...\r\n");
        rt_thread_mdelay(100);
        board_enter_power_down();
    }
    else if (rt_strncmp(cmd, "deep_powerdown", 14) == 0) {
        rt_kprintf("Entering deep power down mode...\r\n");
        rt_thread_mdelay(100);
        board_enter_deep_powerdown();
    }
    else if (rt_strncmp(cmd, "help", 4) == 0) {
        rt_kprintf("Available commands:\r\n");
        rt_kprintf("  sleep          - Enter sleep mode\r\n");
        rt_kprintf("  deep_sleep     - Enter deep sleep mode\r\n");
        rt_kprintf("  power_down     - Enter power down mode\r\n");
        rt_kprintf("  deep_powerdown - Enter deep power down mode\r\n");
        rt_kprintf("  help           - Show this help message\r\n");
    }
    else if (rt_strlen(cmd) > 0) {
        rt_kprintf("Unknown command: %s\r\n", cmd);
    }
}

/**
 * @brief Handle received character
 * @param ch: Received character
 */
static void uart_handle_char(char ch)
{
    if (ch == '\r' || ch == '\n') {
        if (uart_cmd_pos > 0) {
            uart_cmd_buffer[uart_cmd_pos] = '\0';
            uart_process_command(uart_cmd_buffer);
            uart_cmd_pos = 0;
        }
    }
    else if (ch == '\b' || ch == 0x7F) {  /* Backspace */
        if (uart_cmd_pos > 0) {
            uart_cmd_pos--;
            rt_kprintf("\b \b");
        }
    }
    else if (uart_cmd_pos < UART_CMD_BUFFER_SIZE - 1) {
        uart_cmd_buffer[uart_cmd_pos++] = ch;
        rt_kprintf("%c", ch);  /* Echo character */
    }
}

#ifdef RT_USING_SERIAL

#ifdef RT_USING_DEVICE
#include <rtdevice.h>
#endif

#define UART_RX_BUFSZ 8

/* LPC8XX uart driver */
struct lpc8xx_uart
{
    struct rt_device parent;
    struct rt_ringbuffer rx_rb;
    LPC_USART_T * uart_base;
    IRQn_Type uart_irq;
    rt_uint8_t rx_buffer[UART_RX_BUFSZ];

};
#ifdef RT_USING_SERIAL0
struct lpc8xx_uart uart0_device;
#endif

#ifdef RT_USING_SERIAL1
struct lpc8xx_uart uart1_device;
#endif

#ifdef RT_USING_SERIAL2
struct lpc8xx_uart uart2_device;
#endif

void uart_irq_handler(struct lpc8xx_uart* uart)
{
    uint32_t status;
    uint8_t ch;

    /* enter interrupt */
    rt_interrupt_enter();

    status = Chip_UART_GetStatus(uart->uart_base);
    if(status & UART_STAT_RXRDY)    // RXIRQ
    {
        ch = (rt_uint8_t)Chip_UART_ReadByte(uart->uart_base);
        
        /* Store character in ring buffer */
        rt_ringbuffer_putchar_force(&(uart->rx_rb), ch);
        
        /* Process command character in real-time */
        uart_handle_char(ch);
        
        /* invoke callback */
        if(uart->parent.rx_indicate != RT_NULL)
        {
            uart->parent.rx_indicate(&uart->parent, rt_ringbuffer_data_len(&uart->rx_rb));
        }
    }

    /* leave interrupt */
    rt_interrupt_leave();
}

#ifdef RT_USING_UART0
void UART0_IRQHandler(void)
{
    uart_irq_handler(&uart0_device);
}
#endif

#ifdef RT_USING_UART1
void UART1_IRQHandler(void)
{
    uart_irq_handler(&uart1_device);
}
#endif

#ifdef RT_USING_UART2
void UART2_IRQHandler(void)
{
    uart_irq_handler(&uart2_device);
}
#endif

static void uart1_io_init(LPC_USART_T * uart_base)
{
    /* Enable the clock to the Switch Matrix */
    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);

    Chip_Clock_SetUARTClockDiv(1);

#ifdef RT_USING_SERIAL0
    if (uart_base == LPC_USART0)
    {
        Chip_SWM_MovablePinAssign(SWM_U0_TXD_O, 4);
        Chip_SWM_MovablePinAssign(SWM_U0_RXD_I, 0);
    }
    else
#endif

#ifdef RT_USING_SERIAL1
    if (uart_base == LPC_USART1)
    {
        Chip_SWM_MovablePinAssign(SWM_U1_TXD_O, 4);
        Chip_SWM_MovablePinAssign(SWM_U1_RXD_I, 0);
    }
    else
#endif

#ifdef RT_USING_SERIAL2
    if (uart_base == LPC_USART2)
    {
        Chip_SWM_MovablePinAssign(SWM_U2_TXD_O, 4);
        Chip_SWM_MovablePinAssign(SWM_U2_RXD_I, 0);
    }
    else
#endif
    {
        RT_ASSERT((uart_base == USART0) || (uart_base == USART2) || (uart_base == USART2));
    }

    /* Disable the clock to the Switch Matrix to save power */
    Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);
}

static void uart_ll_init(LPC_USART_T * uart)
{
    Chip_UART_Init(uart);
    Chip_UART_ConfigData(uart, UART_CFG_DATALEN_8 | UART_CFG_PARITY_NONE | UART_CFG_STOPLEN_1);
    Chip_Clock_SetUSARTNBaseClockRate((RT_SERIAL_BAUDRATE * 6 * 16), true);
    Chip_UART_SetBaud(uart, RT_SERIAL_BAUDRATE);
    Chip_UART_Enable(uart);
    Chip_UART_TXEnable(uart);

    // we must NOT enable TX ready/idle IRQ before we want to write data
    // otherwise the IRQs will happen as soon as Uart IRQ is enabled in NVIC
    Chip_UART_IntDisable(uart, UART_INTEN_TXRDY | UART_INTEN_TXIDLE);
    Chip_UART_IntEnable(uart, UART_INTEN_RXRDY);
}

static rt_err_t rt_uart_init (rt_device_t dev)
{
    struct lpc8xx_uart* uart;
    RT_ASSERT(dev != RT_NULL);
    uart = (struct lpc8xx_uart *)dev;

    uart1_io_init(uart->uart_base);
    uart_ll_init(uart->uart_base);

    return RT_EOK;
}

static rt_err_t rt_uart_open(rt_device_t dev, rt_uint16_t oflag)
{
    struct lpc8xx_uart* uart;
    RT_ASSERT(dev != RT_NULL);
    uart = (struct lpc8xx_uart *)dev;

    if (dev->flag & RT_DEVICE_FLAG_INT_RX)
    {
        /* Enable the UART Interrupt */
        NVIC_EnableIRQ(uart->uart_irq);
    }

    return RT_EOK;
}

static rt_err_t rt_uart_close(rt_device_t dev)
{
    struct lpc8xx_uart* uart;
    RT_ASSERT(dev != RT_NULL);
    uart = (struct lpc8xx_uart *)dev;

    if (dev->flag & RT_DEVICE_FLAG_INT_RX)
    {
        /* Disable the UART Interrupt */
        NVIC_DisableIRQ(uart->uart_irq);
    }

    return RT_EOK;
}

static rt_ssize_t rt_uart_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    /* interrupt receive */
    rt_base_t level;

    rt_size_t length;
    struct lpc8xx_uart* uart;
    RT_ASSERT(dev != RT_NULL);
    uart = (struct lpc8xx_uart *)dev;


    RT_ASSERT(uart != RT_NULL);

    /* disable interrupt */
    level = rt_hw_interrupt_disable();
    length = rt_ringbuffer_get(&(uart->rx_rb), buffer, size);
    /* enable interrupt */
    rt_hw_interrupt_enable(level);

    return length;
}

static rt_ssize_t rt_uart_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
    char *ptr = (char*) buffer;
    struct lpc8xx_uart* uart;
    RT_ASSERT(dev != RT_NULL);
    uart = (struct lpc8xx_uart *)dev;

    if (dev->open_flag & RT_DEVICE_FLAG_STREAM)
    {
        /* stream mode */
        while (size)
        {
            if (*ptr == '\n')
            {
                while (!(Chip_UART_GetStatus(uart->uart_base) & UART_STAT_TXRDY));
                Chip_UART_SendByte(uart->uart_base, '\r');
            }

                while (!(Chip_UART_GetStatus(uart->uart_base) & UART_STAT_TXRDY));
            Chip_UART_SendByte(uart->uart_base, *ptr);

            ptr ++;
            size --;
        }
    }
    else
    {
        while (size)
        {
            while (!(Chip_UART_GetStatus(uart->uart_base) & UART_STAT_TXRDY));
            Chip_UART_SendByte(uart->uart_base, *ptr);

            ptr++;
            size--;
        }
    }

    return (rt_size_t) ptr - (rt_size_t) buffer;
}

int rt_hw_usart_init(void)
{
#ifdef RT_USING_SERIAL0
    {
        struct lpc8xx_uart* uart;

        /* get uart device */
        uart = &uart0_device;

        /* device initialization */
        uart->parent.type = RT_Device_Class_Char;
        uart->uart_base = LPC_USART0;
        uart->uart_irq = UART0_IRQn;

        rt_ringbuffer_init(&(uart->rx_rb), uart->rx_buffer, sizeof(uart->rx_buffer));

        /* device interface */
        uart->parent.init       = rt_uart_init;
        uart->parent.open       = rt_uart_open;
        uart->parent.close      = rt_uart_close;
        uart->parent.read       = rt_uart_read;
        uart->parent.write      = rt_uart_write;
        uart->parent.control    = RT_NULL;
        uart->parent.user_data  = RT_NULL;

        rt_device_register(&uart->parent, "uart0", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    }
#endif

#ifdef RT_USING_SERIAL1
    {
        struct lpc8xx_uart* uart;

        /* get uart device */
        uart = &uart1_device;

        /* device initialization */
        uart->parent.type = RT_Device_Class_Char;
        uart->uart_base = LPC_USART1;
        uart->uart_irq = UART1_IRQn;

        rt_ringbuffer_init(&(uart->rx_rb), uart->rx_buffer, sizeof(uart->rx_buffer));

        /* device interface */
        uart->parent.init       = rt_uart_init;
        uart->parent.open       = rt_uart_open;
        uart->parent.close      = rt_uart_close;
        uart->parent.read       = rt_uart_read;
        uart->parent.write      = rt_uart_write;
        uart->parent.control    = RT_NULL;
        uart->parent.user_data  = RT_NULL;

        rt_device_register(&uart->parent, "uart1", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    }
#endif

#ifdef RT_USING_SERIAL2
    {
        struct lpc8xx_uart* uart;

        /* get uart device */
        uart = &uart2_device;

        /* device initialization */
        uart->parent.type = RT_Device_Class_Char;
        uart->uart_base = LPC_USART2;
        uart->uart_irq = UART2_IRQn;
        rt_ringbuffer_init(&(uart->rx_rb), uart->rx_buffer, sizeof(uart->rx_buffer));

        /* device interface */
        uart->parent.init       = rt_uart_init;
        uart->parent.open       = rt_uart_open;
        uart->parent.close      = rt_uart_close;
        uart->parent.read       = rt_uart_read;
        uart->parent.write      = rt_uart_write;
        uart->parent.control    = RT_NULL;
        uart->parent.user_data  = RT_NULL;

        rt_device_register(&uart->parent, "uart2", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    }
#endif /* RT_USING_SERIAL2 */
    return 0;
}
INIT_BOARD_EXPORT(rt_hw_usart_init);

#endif /*RT_USING_SERIAL*/
