/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * SPI driver header for LPC824
 */

#ifndef __DRV_SPI_H__
#define __DRV_SPI_H__

#include <rtthread.h>
#include <rtdevice.h>

#define SPI_BUS_NAME    "spi0"

/* SPI pin definitions */
#define SPI_MOSI_PORT   0
#define SPI_MOSI_PIN    25

#define SPI_MISO_PORT   0
#define SPI_MISO_PIN    24

#define SPI_SCK_PORT    0
#define SPI_SCK_PIN     26

#define SPI_CS_PORT     0
#define SPI_CS_PIN      15

int rt_hw_spi_init(void);

#endif /* __DRV_SPI_H__ */
