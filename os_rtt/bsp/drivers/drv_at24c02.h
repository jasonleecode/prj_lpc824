/*
 * AT24C02 EEPROM Driver Header
 * I2C EEPROM 2KB (256 bytes x 8 bits)
 */

#ifndef __DRV_AT24C02_H__
#define __DRV_AT24C02_H__

#include <rtthread.h>
#include <rtdevice.h>

/* AT24C02 I2C address (7-bit) */
#define AT24C02_I2C_ADDR    0x50

/* AT24C02 memory size */
#define AT24C02_SIZE        256

/* AT24C02 page size for write operations */
#define AT24C02_PAGE_SIZE   8

/* Write cycle time (max 5ms) */
#define AT24C02_WRITE_TIME  6

/* Function declarations */
rt_err_t at24c02_init(void);
rt_err_t at24c02_read(uint16_t addr, uint8_t *buf, rt_size_t size);
rt_err_t at24c02_write(uint16_t addr, const uint8_t *buf, rt_size_t size);

#endif /* __DRV_AT24C02_H__ */