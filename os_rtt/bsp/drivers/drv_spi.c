/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * SPI driver implementation for LPC824
 * Supports SPI0 bus with RT-Thread SPI interface
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_spi.h"
#include "peri_driver.h"

#ifdef RT_USING_SPI

/* SPI device structure */
struct lpc8xx_spi_device
{
    struct rt_spi_bus spi_bus;
    LPC_SPI_T *spi_base;
    struct rt_spi_configuration cfg;
};

static struct lpc8xx_spi_device lpc8xx_spi0;

/* SPI interrupt handler */
void SPI0_IRQHandler(void)
{
    rt_interrupt_enter();
    
    /* Handle SPI interrupt */
    uint32_t status = Chip_SPI_GetStatus(lpc8xx_spi0.spi_base);
    
    if (status & SPI_STAT_RXRDY)
    {
        /* Read received data */
        Chip_SPI_ReadFrames8Bits(lpc8xx_spi0.spi_base, NULL, 1);
    }
    
    rt_interrupt_leave();
}

/**
 * SPI configure function
 */
static rt_err_t spi_configure(struct rt_spi_device *device,
                             struct rt_spi_configuration *configuration)
{
    struct lpc8xx_spi_device *dev = (struct lpc8xx_spi_device *)device->bus->parent.user_data;
    
    if (dev == RT_NULL || configuration == RT_NULL)
        return -RT_EINVAL;
    
    /* Store configuration */
    rt_memcpy(&dev->cfg, configuration, sizeof(struct rt_spi_configuration));
    
    /* Configure SPI mode (CPHA, CPOL) */
    uint32_t mode = 0;
    if (configuration->mode & RT_SPI_CPHA)
        mode |= SPI_CFG_CPHA_2ND;
    if (configuration->mode & RT_SPI_CPOL)
        mode |= SPI_CFG_CPOL_HIGH;
    
    /* Configure SPI data width */
    uint32_t datawidth = configuration->data_width;
    
    /* Setup SPI configuration */
    Chip_SPI_SetMaster(dev->spi_base, true);
    Chip_SPI_SetupXfer(dev->spi_base, mode | SPI_CFG_MSB_FIRST_EN, 
                       datawidth, configuration->max_hz);
    
    return RT_EOK;
}

/**
 * SPI transmit and receive function
 */
static rt_uint32_t spi_xfer(struct rt_spi_device *device, struct rt_spi_message *message)
{
    struct lpc8xx_spi_device *dev = (struct lpc8xx_spi_device *)device->bus->parent.user_data;
    rt_uint32_t size = message->length;
    rt_uint8_t *send_ptr = (rt_uint8_t *)message->send_buf;
    rt_uint8_t *recv_ptr = (rt_uint8_t *)message->recv_buf;
    rt_uint32_t i;
    
    if (dev == RT_NULL)
        return 0;
    
    for (i = 0; i < size; i++)
    {
        rt_uint32_t data = 0xFF;
        
        if (send_ptr != RT_NULL)
            data = send_ptr[i];
        
        /* Send byte */
        Chip_SPI_SendMidFrame(dev->spi_base, data);
        
        /* Wait for data ready */
        while (!(Chip_SPI_GetStatus(dev->spi_base) & SPI_STAT_RXRDY));
        
        /* Receive byte */
        data = Chip_SPI_ReadFrames8Bits(dev->spi_base, NULL, 1);
        
        if (recv_ptr != RT_NULL)
            recv_ptr[i] = data;
    }
    
    return size;
}

static const struct rt_spi_ops spi_ops =
{
    spi_configure,
    spi_xfer
};

/**
 * Initialize SPI peripheral pins
 */
static void spi_pin_init(void)
{
    /* Enable the clock to the Switch Matrix */
    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);
    
    /* Enable SPI0 clock */
    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SPI0);
    
    /* Assign SPI pins: MOSI, MISO, SCK */
    Chip_SWM_MovablePinAssign(SWM_SPI0_MOSI_IO, SPI_MOSI_PIN);
    Chip_SWM_MovablePinAssign(SWM_SPI0_MISO_IO, SPI_MISO_PIN);
    Chip_SWM_MovablePinAssign(SWM_SPI0_SCK_IO, SPI_SCK_PIN);
    
    /* Configure CS pin as GPIO output */
    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, SPI_CS_PORT, SPI_CS_PIN);
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, SPI_CS_PORT, SPI_CS_PIN, true);
    
    /* Disable the clock to the Switch Matrix */
    Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);
}

/**
 * RT-Thread SPI bus initialization
 */
int rt_hw_spi_init(void)
{
    /* Initialize SPI pins */
    spi_pin_init();
    
    /* Initialize SPI0 */
    Chip_SPI_Init(LPC_SPI0);
    
    /* Setup SPI0 device structure */
    lpc8xx_spi0.spi_base = LPC_SPI0;
    lpc8xx_spi0.spi_bus.parent.user_data = (void *)&lpc8xx_spi0;
    lpc8xx_spi0.spi_bus.ops = &spi_ops;
    
    /* Register SPI0 bus device */
    rt_spi_bus_attach_device(&lpc8xx_spi0.spi_bus, SPI_BUS_NAME, RT_NULL);
    
    return RT_EOK;
}

INIT_BOARD_EXPORT(rt_hw_spi_init);

#endif /* RT_USING_SPI */
