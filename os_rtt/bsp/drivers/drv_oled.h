/*
 * OLED Display Driver (SSD1306)
 * 128x64 monochrome OLED display via I2C
 */

#ifndef __DRV_OLED_H__
#define __DRV_OLED_H__

#include <rtthread.h>
#include <rtdevice.h>

/* OLED display dimensions */
#define OLED_WIDTH      128
#define OLED_HEIGHT     64

/* OLED I2C address (7-bit) */
#define OLED_I2C_ADDR   0x3C

/* OLED control byte */
#define OLED_CTRL_CMD   0x00
#define OLED_CTRL_DATA  0x40

/* Function declarations */
rt_err_t oled_init(void);
rt_err_t oled_clear(void);
rt_err_t oled_display_on(void);
rt_err_t oled_display_off(void);
rt_err_t oled_write_pixel(uint8_t x, uint8_t y, uint8_t pixel);
rt_err_t oled_write_string(uint8_t x, uint8_t y, const char *str);

#endif /* __DRV_OLED_H__ */
