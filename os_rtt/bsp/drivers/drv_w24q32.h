/*
 * W25Q32 SPI Flash Driver Header
 * 4MB SPI Flash Memory (W25Q32)
 */

#ifndef __DRV_W25Q32_H__
#define __DRV_W25Q32_H__

#include <rtthread.h>
#include <rtdevice.h>

/* W25Q32 capacity: 4MB (32Mbit) */
#define W25Q32_SIZE         (4 * 1024 * 1024)

/* W25Q32 page size: 256 bytes */
#define W25Q32_PAGE_SIZE    256

/* W25Q32 sector size: 4KB */
#define W25Q32_SECTOR_SIZE  (4 * 1024)

/* W25Q32 block size: 64KB */
#define W25Q32_BLOCK_SIZE   (64 * 1024)

/* W25Q32 SPI commands */
#define W25Q32_CMD_READ             0x03
#define W25Q32_CMD_FAST_READ        0x0B
#define W25Q32_CMD_PAGE_PROGRAM     0x02
#define W25Q32_CMD_SECTOR_ERASE     0x20
#define W25Q32_CMD_BLOCK_ERASE      0xD8
#define W25Q32_CMD_CHIP_ERASE       0xC7
#define W25Q32_CMD_READ_STATUS      0x05
#define W25Q32_CMD_WRITE_ENABLE     0x06
#define W25Q32_CMD_WRITE_DISABLE    0x04
#define W25Q32_CMD_READ_ID          0x9F

/* Function declarations */
rt_err_t w25q32_init(void);
rt_err_t w25q32_read(uint32_t addr, uint8_t *buf, rt_size_t size);
rt_err_t w25q32_write(uint32_t addr, const uint8_t *buf, rt_size_t size);
rt_err_t w25q32_erase_sector(uint32_t addr);

#endif /* __DRV_W25Q32_H__ */
