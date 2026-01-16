/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * I2C driver implementation for LPC824
 * Supports I2C0 bus with RT-Thread I2C interface
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_i2c.h"
#include "peri_driver.h"

#ifdef RT_USING_I2C

/* I2C device structure */
struct lpc8xx_i2c_device
{
    struct rt_i2c_bus_device i2c_bus;
    LPC_I2C_T *i2c_base;
    I2CM_XFER_T xfer;
};

static struct lpc8xx_i2c_device lpc8xx_i2c0;

/* I2C interrupt handler for I2C0 */
void I2C0_IRQHandler(void)
{
    rt_interrupt_enter();
    
    /* Handle I2C interrupt */
    Chip_I2CM_XferHandler(LPC_I2CM, &lpc8xx_i2c0.xfer);
    
    rt_interrupt_leave();
}

/**
 * I2C transfer function (master mode)
 */
static rt_ssize_t i2c_transfer(struct rt_i2c_bus_device *bus,
                              struct rt_i2c_msg msgs[], rt_uint32_t num)
{
    struct lpc8xx_i2c_device *dev = (struct lpc8xx_i2c_device *)bus;
    rt_uint32_t i;
    
    if (dev == RT_NULL || msgs == RT_NULL || num == 0)
        return -RT_EINVAL;
    
    for (i = 0; i < num; i++)
    {
        I2CM_XFER_T xfer;
        
        xfer.slaveAddr = msgs[i].addr;
        xfer.rxBuff = (uint8_t *)msgs[i].buf;
        xfer.rxSz = msgs[i].len;
        
        if (msgs[i].flags & RT_I2C_WR)
        {
            xfer.rxBuff = RT_NULL;
            xfer.rxSz = 0;
            xfer.txBuff = (uint8_t *)msgs[i].buf;
            xfer.txSz = msgs[i].len;
        }
        
        /* Start transfer */
        Chip_I2CM_Xfer(dev->i2c_base, &xfer);
    }
    
    return (rt_ssize_t)num;
}

static const struct rt_i2c_bus_device_ops i2c_ops =
{
    i2c_transfer,
    RT_NULL,
    RT_NULL,
};

/**
 * Initialize I2C peripheral and register as RT-Thread I2C bus device
 */
static void i2c_pin_init(void)
{
    /* Enable the clock to the Switch Matrix */
    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);
    
    /* Enable I2C0 clock */
    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_I2C0);
    
    /* Configure I2C0 pins (I2C_SDA on P0.23, I2C_SCL on P0.24) */
    Chip_SWM_MovablePinAssign(SWM_I2C_SDA_IO, 23);
    Chip_SWM_MovablePinAssign(SWM_I2C_SCL_IO, 24);
    
    /* Disable the clock to the Switch Matrix */
    Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);
}

/**
 * RT-Thread I2C bus initialization
 */
int rt_hw_i2c_init(void)
{
    /* Initialize I2C pins */
    i2c_pin_init();
    
    /* Initialize I2C0 master with 400KHz clock */
    Chip_I2CM_Init(LPC_I2CM, I2C_SPEED_400K);
    
    /* Setup I2C0 device structure */
    lpc8xx_i2c0.i2c_base = LPC_I2CM;
    lpc8xx_i2c0.i2c_bus.ops = &i2c_ops;
    
    /* Register I2C0 bus device */
    rt_i2c_bus_device_register(&lpc8xx_i2c0.i2c_bus, I2C_BUS_NAME);
    
    return RT_EOK;
}

INIT_BOARD_EXPORT(rt_hw_i2c_init);

#endif /* RT_USING_I2C */
