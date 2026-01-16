/*
 * Button/Key Driver Implementation for LPC824
 * GPIO-based button input with debouncing
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_key.h"
#include "peri_driver.h"

/* Key callback function pointer */
static void (*key_callback)(uint8_t key_code) = RT_NULL;

/**
 * Initialize GPIO pins for keys
 */
static void key_gpio_init(void)
{
    /* Configure key pins as inputs */
    Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, KEY_UP_PORT, KEY_UP_PIN);
    Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, KEY_DOWN_PORT, KEY_DOWN_PIN);
    Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, KEY_LEFT_PORT, KEY_LEFT_PIN);
    Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, KEY_RIGHT_PORT, KEY_RIGHT_PIN);
}

/**
 * Initialize button driver
 */
rt_err_t key_init(void)
{
    /* Initialize GPIO for keys */
    key_gpio_init();
    
    rt_kprintf("KEY: Driver initialized\n");
    return RT_EOK;
}

/**
 * Read key state with debouncing
 * Returns key code (0 if no key pressed)
 */
uint8_t key_read(void)
{
    uint8_t key_code = 0;
    
    /* Check UP key */
    if (!Chip_GPIO_GetPinState(LPC_GPIO_PORT, KEY_UP_PORT, KEY_UP_PIN))
    {
        rt_thread_mdelay(KEY_DEBOUNCE_MS);
        if (!Chip_GPIO_GetPinState(LPC_GPIO_PORT, KEY_UP_PORT, KEY_UP_PIN))
            key_code = KEY_CODE_UP;
    }
    
    /* Check DOWN key */
    if (!Chip_GPIO_GetPinState(LPC_GPIO_PORT, KEY_DOWN_PORT, KEY_DOWN_PIN))
    {
        rt_thread_mdelay(KEY_DEBOUNCE_MS);
        if (!Chip_GPIO_GetPinState(LPC_GPIO_PORT, KEY_DOWN_PORT, KEY_DOWN_PIN))
            key_code = KEY_CODE_DOWN;
    }
    
    /* Check LEFT key */
    if (!Chip_GPIO_GetPinState(LPC_GPIO_PORT, KEY_LEFT_PORT, KEY_LEFT_PIN))
    {
        rt_thread_mdelay(KEY_DEBOUNCE_MS);
        if (!Chip_GPIO_GetPinState(LPC_GPIO_PORT, KEY_LEFT_PORT, KEY_LEFT_PIN))
            key_code = KEY_CODE_LEFT;
    }
    
    /* Check RIGHT key */
    if (!Chip_GPIO_GetPinState(LPC_GPIO_PORT, KEY_RIGHT_PORT, KEY_RIGHT_PIN))
    {
        rt_thread_mdelay(KEY_DEBOUNCE_MS);
        if (!Chip_GPIO_GetPinState(LPC_GPIO_PORT, KEY_RIGHT_PORT, KEY_RIGHT_PIN))
            key_code = KEY_CODE_RIGHT;
    }
    
    return key_code;
}

/**
 * Register key event callback
 */
rt_err_t key_register_callback(void (*callback)(uint8_t key_code))
{
    if (callback == RT_NULL)
        return -RT_EINVAL;
    
    key_callback = callback;
    return RT_EOK;
}

/**
 * Key scan thread
 * Periodically checks for key presses
 */
static void key_thread_entry(void *parameter)
{
    uint8_t key_code;
    
    while (1)
    {
        key_code = key_read();
        
        if (key_code != 0 && key_callback != RT_NULL)
        {
            key_callback(key_code);
        }
        
        rt_thread_mdelay(50);
    }
}

/**
 * Create key scan thread
 */
static int key_thread_init(void)
{
    rt_thread_t thread;
    
    key_init();
    
    thread = rt_thread_create("key_scan",
                            key_thread_entry,
                            RT_NULL,
                            1024,
                            10,
                            10);
    
    if (thread != RT_NULL)
        rt_thread_startup(thread);
    
    return RT_EOK;
}

INIT_APP_EXPORT(key_thread_init);
