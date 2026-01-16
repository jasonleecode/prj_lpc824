/*
 * OLED Display Driver (SSD1306) Implementation
 * 128x64 monochrome OLED display via I2C
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include "drv_oled.h"
#include "drv_i2c.h"

/* I2C bus device handle */
static struct rt_i2c_bus_device *i2c_bus = RT_NULL;

/* Display buffer (128x8 bytes = 1024 bytes) */
static uint8_t oled_buffer[OLED_WIDTH * OLED_HEIGHT / 8];

/**
 * Send command to OLED
 */
static rt_err_t oled_send_cmd(uint8_t cmd)
{
    struct rt_i2c_msg msg;
    uint8_t buf[2];
    
    if (i2c_bus == RT_NULL)
        return -RT_ENOSYS;
    
    buf[0] = OLED_CTRL_CMD;  /* Control byte for command */
    buf[1] = cmd;
    
    msg.addr = OLED_I2C_ADDR;
    msg.flags = RT_I2C_WR;
    msg.buf = buf;
    msg.len = 2;
    
    if (rt_i2c_transfer(i2c_bus, &msg, 1) != 1)
        return -RT_EIO;
    
    return RT_EOK;
}

/**
 * Send data to OLED
 */
static rt_err_t oled_send_data(uint8_t *data, rt_size_t len)
{
    struct rt_i2c_msg msg;
    uint8_t buf[len + 1];
    
    if (i2c_bus == RT_NULL)
        return -RT_ENOSYS;
    
    buf[0] = OLED_CTRL_DATA;  /* Control byte for data */
    rt_memcpy(&buf[1], data, len);
    
    msg.addr = OLED_I2C_ADDR;
    msg.flags = RT_I2C_WR;
    msg.buf = buf;
    msg.len = len + 1;
    
    if (rt_i2c_transfer(i2c_bus, &msg, 1) != 1)
        return -RT_EIO;
    
    return RT_EOK;
}

/**
 * Initialize OLED display (SSD1306)
 */
rt_err_t oled_init(void)
{
    /* Find I2C bus device */
    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(I2C_BUS_NAME);
    if (i2c_bus == RT_NULL)
    {
        rt_kprintf("OLED: I2C bus not found\n");
        return -RT_ENOSYS;
    }
    
    /* Initialize SSD1306 controller */
    rt_thread_mdelay(100);
    
    /* Display OFF */
    oled_send_cmd(0xAE);
    
    /* Set clock divide ratio and oscillator frequency */
    oled_send_cmd(0xD5);
    oled_send_cmd(0x80);
    
    /* Set multiplex ratio */
    oled_send_cmd(0xA8);
    oled_send_cmd(0x3F);
    
    /* Set display offset */
    oled_send_cmd(0xD3);
    oled_send_cmd(0x00);
    
    /* Set start line address */
    oled_send_cmd(0x40);
    
    /* Charge pump setting */
    oled_send_cmd(0x8D);
    oled_send_cmd(0x14);
    
    /* Memory addressing mode */
    oled_send_cmd(0x20);
    oled_send_cmd(0x02);
    
    /* Segment remap */
    oled_send_cmd(0xA1);
    
    /* COM output scan direction */
    oled_send_cmd(0xC8);
    
    /* COM pins configuration */
    oled_send_cmd(0xDA);
    oled_send_cmd(0x12);
    
    /* Contrast control */
    oled_send_cmd(0x81);
    oled_send_cmd(0xCF);
    
    /* Pre-charge period */
    oled_send_cmd(0xD9);
    oled_send_cmd(0xF1);
    
    /* VCOMH deselect level */
    oled_send_cmd(0xDB);
    oled_send_cmd(0x40);
    
    /* Display ON */
    oled_send_cmd(0xAF);
    
    /* Clear display */
    oled_clear();
    
    rt_kprintf("OLED: Display initialized\n");
    return RT_EOK;
}

/**
 * Clear OLED display
 */
rt_err_t oled_clear(void)
{
    rt_size_t i;
    
    if (i2c_bus == RT_NULL)
        return -RT_ENOSYS;
    
    /* Clear display buffer */
    rt_memset(oled_buffer, 0, sizeof(oled_buffer));
    
    /* Send clear data to display */
    for (i = 0; i < OLED_HEIGHT / 8; i++)
    {
        oled_send_cmd(0xB0 + i);  /* Set page address */
        oled_send_cmd(0x00);      /* Set column address lower byte */
        oled_send_cmd(0x10);      /* Set column address upper byte */
        oled_send_data(&oled_buffer[i * OLED_WIDTH], OLED_WIDTH);
    }
    
    return RT_EOK;
}

/**
 * Turn OLED display ON
 */
rt_err_t oled_display_on(void)
{
    if (i2c_bus == RT_NULL)
        return -RT_ENOSYS;
    
    return oled_send_cmd(0xAF);
}

/**
 * Turn OLED display OFF
 */
rt_err_t oled_display_off(void)
{
    if (i2c_bus == RT_NULL)
        return -RT_ENOSYS;
    
    return oled_send_cmd(0xAE);
}

/**
 * Write pixel to display buffer
 */
rt_err_t oled_write_pixel(uint8_t x, uint8_t y, uint8_t pixel)
{
    rt_uint16_t offset;
    
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT)
        return -RT_EINVAL;
    
    offset = (y / 8) * OLED_WIDTH + x;
    
    if (pixel)
        oled_buffer[offset] |= (1 << (y % 8));
    else
        oled_buffer[offset] &= ~(1 << (y % 8));
    
    return RT_EOK;
}

/**
 * Write ASCII string to display
 * Simplified version - writes at given position
 */
rt_err_t oled_write_string(uint8_t x, uint8_t y, const char *str)
{
    if (i2c_bus == RT_NULL || str == RT_NULL)
        return -RT_ENOSYS;
    
    /* Set position and send string */
    oled_send_cmd(0xB0 + (y / 8));  /* Set page */
    oled_send_cmd((x & 0x0F));      /* Set column lower */
    oled_send_cmd((x >> 4) | 0x10); /* Set column upper */
    
    /* In a real implementation, you would render the font here */
    rt_kprintf("OLED: String at (%d, %d): %s\n", x, y, str);
    
    return RT_EOK;
}
