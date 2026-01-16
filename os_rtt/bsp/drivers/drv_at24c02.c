/*
 * AT24C02 EEPROM Driver Implementation
 * 2KB I2C EEPROM for LPC824
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include "drv_at24c02.h"
#include "drv_i2c.h"

/* I2C bus device handle */
static struct rt_i2c_bus_device *i2c_bus = RT_NULL;

/**
 * Initialize AT24C02 EEPROM
 * Locate I2C bus and verify device presence
 */
rt_err_t at24c02_init(void)
{
    struct rt_i2c_msg msg;
    uint8_t test_byte = 0;
    
    /* Find I2C bus device */
    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(I2C_BUS_NAME);
    if (i2c_bus == RT_NULL)
    {
        rt_kprintf("AT24C02: I2C bus '%s' not found\n", I2C_BUS_NAME);
        return -RT_ENOSYS;
    }
    
    /* Try to read one byte to verify device presence */
    msg.addr = AT24C02_I2C_ADDR;
    msg.flags = RT_I2C_RD;
    msg.buf = &test_byte;
    msg.len = 1;
    
    if (rt_i2c_transfer(i2c_bus, &msg, 1) != 1)
    {
        rt_kprintf("AT24C02: Device not found at address 0x%02X\n", AT24C02_I2C_ADDR);
        return -RT_ENODEV;
    }
    
    rt_kprintf("AT24C02: Device initialized successfully\n");
    return RT_EOK;
}

/**
 * Read data from AT24C02 EEPROM
 * @addr: Start address (0-255)
 * @buf: Buffer to store read data
 * @size: Number of bytes to read
 */
rt_err_t at24c02_read(uint16_t addr, uint8_t *buf, rt_size_t size)
{
    struct rt_i2c_msg msgs[2];
    uint8_t addr_byte;
    
    if (i2c_bus == RT_NULL)
    {
        rt_kprintf("AT24C02: Not initialized\n");
        return -RT_ENOSYS;
    }
    
    if (addr >= AT24C02_SIZE || (addr + size) > AT24C02_SIZE)
    {
        rt_kprintf("AT24C02: Invalid address range\n");
        return -RT_EINVAL;
    }
    
    addr_byte = (uint8_t)addr;
    
    /* First message: write address byte */
    msgs[0].addr = AT24C02_I2C_ADDR;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf = &addr_byte;
    msgs[0].len = 1;
    
    /* Second message: read data */
    msgs[1].addr = AT24C02_I2C_ADDR;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf = buf;
    msgs[1].len = size;
    
    if (rt_i2c_transfer(i2c_bus, msgs, 2) != 2)
    {
        rt_kprintf("AT24C02: Read failed at address 0x%02X\n", addr);
        return -RT_EIO;
    }
    
    return RT_EOK;
}

/**
 * Write data to AT24C02 EEPROM
 * Note: Writing is done page by page (max 8 bytes per write)
 * @addr: Start address (0-255)
 * @buf: Data to write
 * @size: Number of bytes to write
 */
rt_err_t at24c02_write(uint16_t addr, const uint8_t *buf, rt_size_t size)
{
    struct rt_i2c_msg msg;
    uint8_t write_buf[AT24C02_PAGE_SIZE + 1];
    rt_size_t bytes_written = 0;
    
    if (i2c_bus == RT_NULL)
    {
        rt_kprintf("AT24C02: Not initialized\n");
        return -RT_ENOSYS;
    }
    
    if (addr >= AT24C02_SIZE || (addr + size) > AT24C02_SIZE)
    {
        rt_kprintf("AT24C02: Invalid address range\n");
        return -RT_EINVAL;
    }
    
    /* Write data page by page */
    while (bytes_written < size)
    {
        rt_size_t page_offset = (addr + bytes_written) & (AT24C02_PAGE_SIZE - 1);
        rt_size_t bytes_in_page = AT24C02_PAGE_SIZE - page_offset;
        rt_size_t bytes_to_write = (size - bytes_written > bytes_in_page) ? 
                                   bytes_in_page : (size - bytes_written);
        
        /* Build write buffer: [address byte] + [data bytes] */
        write_buf[0] = (uint8_t)(addr + bytes_written);
        rt_memcpy(&write_buf[1], &buf[bytes_written], bytes_to_write);
        
        /* Send write command */
        msg.addr = AT24C02_I2C_ADDR;
        msg.flags = RT_I2C_WR;
        msg.buf = write_buf;
        msg.len = bytes_to_write + 1;
        
        if (rt_i2c_transfer(i2c_bus, &msg, 1) != 1)
        {
            rt_kprintf("AT24C02: Write failed at address 0x%02X\n", addr + bytes_written);
            return -RT_EIO;
        }
        
        /* Wait for write cycle to complete */
        rt_thread_mdelay(AT24C02_WRITE_TIME);
        
        bytes_written += bytes_to_write;
    }
    
    return RT_EOK;
}
