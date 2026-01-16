/*
 * Button/Key Driver for LPC824
 * GPIO-based button input handling
 */

#ifndef __DRV_KEY_H__
#define __DRV_KEY_H__

#include <rtthread.h>
#include <rtdevice.h>

/* Key/Button definitions */
#define KEY_UP_PORT     0
#define KEY_UP_PIN      12

#define KEY_DOWN_PORT   0
#define KEY_DOWN_PIN    13

#define KEY_LEFT_PORT   0
#define KEY_LEFT_PIN    20

#define KEY_RIGHT_PORT  0
#define KEY_RIGHT_PIN   19

/* Key codes */
#define KEY_CODE_UP     1
#define KEY_CODE_DOWN   2
#define KEY_CODE_LEFT   3
#define KEY_CODE_RIGHT  4

/* Debounce time (ms) */
#define KEY_DEBOUNCE_MS 20

/* Function declarations */
rt_err_t key_init(void);
uint8_t key_read(void);
rt_err_t key_register_callback(void (*callback)(uint8_t key_code));

#endif /* __DRV_KEY_H__ */
