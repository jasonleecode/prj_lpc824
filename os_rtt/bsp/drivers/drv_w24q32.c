/*
 * W25Q32 SPI Flash Driver Implementation
 * 4MB SPI Flash for LPC824
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include "drv_w24q32.h"
#include "drv_spi.h"

/* SPI device handle */
static struct rt_spi_device *spi_device = RT_NULL;

/**
 * Send command to W25Q32
 */
static void w25q32_send_cmd(uint8_t cmd, uint8_t *data, rt_size_t len)
{
    struct rt_spi_message msg;
    uint8_t buf[len + 1];
    
    buf[0] = cmd;
    if (data && len > 0)
        rt_memcpy(&buf[1], data, len);
    
    msg.send_buf = buf;
    msg.recv_buf = buf;
    msg.length = len + 1;
    msg.cs_take = 1;
    msg.cs_release = 1;
    
    rt_spi_transfer_message(spi_device, &msg);
    
    if (data && len > 0)
        rt_memcpy(data, &buf[1], len);
}

/**
 * Read W25Q32 status register
 */
static uint8_t w25q32_read_status(void)
{
    uint8_t status = 0;
    w25q32_send_cmd(W25Q32_CMD_READ_STATUS, &status, 1);
    return status;
}

/**
 * Wait for W25Q32 to be ready
 */
static void w25q32_wait_ready(void)
{
    while (w25q32_read_status() & 0x01)
        rt_thread_mdelay(1);
}

/**
 * Initialize W25Q32 SPI Flash
 */
rt_err_t w25q32_init(void)
{
    /* Find SPI device */
    spi_device = (struct rt_spi_device *)rt_device_find(SPI_BUS_NAME);
    if (spi_device == RT_NULL)
    {
        rt_kprintf("W25Q32: SPI device not found\n");
        return -RT_ENOSYS;
    }
    
    /* Configure SPI (8-bit data, mode 0, 5MHz) */
    struct rt_spi_configuration cfg;
    cfg.mode = RT_SPI_MODE_0;
    cfg.data_width = 8;
    cfg.max_hz = 5 * 1000 * 1000;  /* 5MHz */
    
    rt_spi_configure(spi_device, &cfg);
    
    rt_kprintf("W25Q32: Device initialized successfully\n");
    return RT_EOK;
}

/**
 * Read data from W25Q32
 * @addr: Start address
 * @buf: Buffer to store read data
 * @size: Number of bytes to read
 */
rt_err_t w25q32_read(uint32_t addr, uint8_t *buf, rt_size_t size)
{
    struct rt_spi_message msg;
    uint8_t cmd_buf[4];
    uint8_t recv_buf[size + 4];
    
    if (spi_device == RT_NULL)
        return -RT_ENOSYS;
    
    if ((addr + size) > W25Q32_SIZE)
        return -RT_EINVAL;
    
    /* Build read command: [CMD(0x03)] [ADDR(3 bytes)] */
    cmd_buf[0] = W25Q32_CMD_READ;
    cmd_buf[1] = (addr >> 16) & 0xFF;
    cmd_buf[2] = (addr >> 8) & 0xFF;
    cmd_buf[3] = addr & 0xFF;
    
    msg.send_buf = cmd_buf;
    msg.recv_buf = recv_buf;
    msg.length = 4;
    msg.cs_take = 1;
    msg.cs_release = 0;
    
    rt_spi_transfer_message(spi_device, &msg);
    
    /* Read data */
    msg.send_buf = RT_NULL;
    msg.recv_buf = buf;
    msg.length = size;
    msg.cs_take = 0;
    msg.cs_release = 1;
    
    rt_spi_transfer_message(spi_device, &msg);
    
    return RT_EOK;
}

/**
 * Write data to W25Q32
 * @addr: Start address
 * @buf: Data to write
 * @size: Number of bytes to write (max 256 bytes per page)
 */
rt_err_t w25q32_write(uint32_t addr, const uint8_t *buf, rt_size_t size)
{
    struct rt_spi_message msg;
    uint8_t cmd_buf[W25Q32_PAGE_SIZE + 4];
    rt_size_t offset = 0;
    
    if (spi_device == RT_NULL)
        return -RT_ENOSYS;
    
    if ((addr + size) > W25Q32_SIZE)
        return -RT_EINVAL;
    
    while (offset < size)
    {
        rt_size_t page_offset = (addr + offset) & (W25Q32_PAGE_SIZE - 1);
        rt_size_t bytes_in_page = W25Q32_PAGE_SIZE - page_offset;
        rt_size_t bytes_to_write = (size - offset > bytes_in_page) ? 
                                    bytes_in_page : (size - offset);
        
        /* Enable write */
        uint8_t cmd = W25Q32_CMD_WRITE_ENABLE;
        msg.send_buf = &cmd;
        msg.recv_buf = RT_NULL;
        msg.length = 1;
        msg.cs_take = 1;
        msg.cs_release = 1;
        rt_spi_transfer_message(spi_device, &msg);
        
        /* Build page program command */
        cmd_buf[0] = W25Q32_CMD_PAGE_PROGRAM;
        cmd_buf[1] = (addr + offset >> 16) & 0xFF;
        cmd_buf[2] = (addr + offset >> 8) & 0xFF;
        cmd_buf[3] = (addr + offset) & 0xFF;
        rt_memcpy(&cmd_buf[4], &buf[offset], bytes_to_write);
        
        msg.send_buf = cmd_buf;
        msg.recv_buf = RT_NULL;
        msg.length = bytes_to_write + 4;
        msg.cs_take = 1;
        msg.cs_release = 1;
        
        rt_spi_transfer_message(spi_device, &msg);
        
        /* Wait for write complete */
        w25q32_wait_ready();
        
        offset += bytes_to_write;
    }
    
    return RT_EOK;
}

/**
 * Erase a 4KB sector
 * @addr: Sector address
 */
rt_err_t w25q32_erase_sector(uint32_t addr)
{
    struct rt_spi_message msg;
    uint8_t cmd_buf[4];
    
    if (spi_device == RT_NULL)
        return -RT_ENOSYS;
    
    if (addr >= W25Q32_SIZE)
        return -RT_EINVAL;
    
    /* Enable write */
    uint8_t cmd = W25Q32_CMD_WRITE_ENABLE;
    msg.send_buf = &cmd;
    msg.recv_buf = RT_NULL;
    msg.length = 1;
    msg.cs_take = 1;
    msg.cs_release = 1;
    rt_spi_transfer_message(spi_device, &msg);
    
    /* Build sector erase command */
    cmd_buf[0] = W25Q32_CMD_SECTOR_ERASE;
    cmd_buf[1] = (addr >> 16) & 0xFF;
    cmd_buf[2] = (addr >> 8) & 0xFF;
    cmd_buf[3] = addr & 0xFF;
    
    msg.send_buf = cmd_buf;
    msg.recv_buf = RT_NULL;
    msg.length = 4;
    msg.cs_take = 1;
    msg.cs_release = 1;
    
    rt_spi_transfer_message(spi_device, &msg);
    
    /* Wait for erase complete */
    w25q32_wait_ready();
    
    return RT_EOK;
}
