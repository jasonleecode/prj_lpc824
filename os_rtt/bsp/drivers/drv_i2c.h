/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * I2C driver for LPC824
 */

#ifndef __DRV_I2C_H__
#define __DRV_I2C_H__

#include <rtthread.h>
#include <rtdevice.h>

/* I2C bus configuration */
#define I2C_BUS_NAME    "i2c0"

/* I2C Master clock speed */
#define I2C_SPEED_100K  100000
#define I2C_SPEED_400K  400000

int rt_hw_i2c_init(void);

#endif /* __DRV_I2C_H__ */
