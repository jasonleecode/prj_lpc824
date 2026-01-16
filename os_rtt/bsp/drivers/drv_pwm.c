/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-16     Jason        LPC824 PWM driver implementation
 */

#include <rtthread.h>
#include "drv_pwm.h"
#include "peri_driver.h"

#ifdef RT_USING_PWM

/* Default PWM channel mapping */
#define PWM_CHANNEL_0       0    /* SCT output 0 */
#define PWM_CHANNEL_1       1    /* SCT output 1 */
#define PWM_CHANNEL_2       2    /* SCT output 2 */
#define PWM_CHANNEL_3       3    /* SCT output 3 */
#define PWM_CHANNEL_4       4    /* SCT output 4 */
#define PWM_CHANNEL_5       5    /* SCT output 5 */

#define PWM_MIN_FREQUENCY   1
#define PWM_MAX_FREQUENCY   10000
#define PWM_DEFAULT_FREQ    1000

/* PWM device instance */
static struct pwm_device pwm_devices[6];
static rt_bool_t pwm_initialized = RT_FALSE;

/**
 * @brief Initialize SCT for PWM mode
 * @param frequency: Desired PWM frequency in Hz
 * @return RT_EOK if success
 */
static rt_err_t pwm_sct_init(rt_uint32_t frequency)
{
    rt_uint32_t sct_tick;
    
    if (frequency < PWM_MIN_FREQUENCY || frequency > PWM_MAX_FREQUENCY) {
        return -RT_EINVAL;
    }
    
    /* Enable SCT clock */
    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SCT);
    
    /* Initialize SCT for PWM */
    Chip_SCTPWM_Init(LPC_SCT);
    
    /* Set SCT PWM rate (frequency in Hz) */
    sct_tick = Chip_Clock_GetSystemClockRate() / frequency;
    Chip_SCTPWM_SetRate(LPC_SCT, sct_tick);
    
    return RT_EOK;
}

/**
 * @brief Initialize PWM output pin
 * @param channel: PWM channel number
 * @return RT_EOK if success
 */
static rt_err_t pwm_pin_init(rt_uint8_t channel)
{
    rt_uint32_t pin_num = 0;
    
    /* Map channel to pin */
    switch (channel) {
        case PWM_CHANNEL_0:
            pin_num = 13;  /* P0.13 */
            break;
        case PWM_CHANNEL_1:
            pin_num = 14;  /* P0.14 */
            break;
        case PWM_CHANNEL_2:
            pin_num = 1;   /* P0.1 */
            break;
        case PWM_CHANNEL_3:
            pin_num = 2;   /* P0.2 */
            break;
        case PWM_CHANNEL_4:
            pin_num = 3;   /* P0.3 */
            break;
        case PWM_CHANNEL_5:
            pin_num = 4;   /* P0.4 */
            break;
        default:
            return -RT_EINVAL;
    }
    
    /* Enable SWM clock */
    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);
    
    /* Configure SCT output pins */
    switch (channel) {
        case PWM_CHANNEL_0:
            Chip_SWM_MovablePinAssign(SWM_SCT_OUT0_O, pin_num);
            break;
        case PWM_CHANNEL_1:
            Chip_SWM_MovablePinAssign(SWM_SCT_OUT1_O, pin_num);
            break;
        case PWM_CHANNEL_2:
            Chip_SWM_MovablePinAssign(SWM_SCT_OUT2_O, pin_num);
            break;
        case PWM_CHANNEL_3:
            Chip_SWM_MovablePinAssign(SWM_SCT_OUT3_O, pin_num);
            break;
        case PWM_CHANNEL_4:
            Chip_SWM_MovablePinAssign(SWM_SCT_OUT4_O, pin_num);
            break;
        case PWM_CHANNEL_5:
            Chip_SWM_MovablePinAssign(SWM_SCT_OUT5_O, pin_num);
            break;
    }
    
    /* Disable SWM clock to save power */
    Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);
    
    return RT_EOK;
}

/**
 * @brief Set PWM duty cycle
 * @param dev: PWM device
 * @param duty_cycle: Duty cycle (0-100%)
 * @return RT_EOK if success
 */
rt_err_t pwm_set_duty_cycle(struct pwm_device *dev, rt_uint8_t duty_cycle)
{
    rt_uint32_t pulse_width;
    
    if (!dev || duty_cycle > 100) {
        return -RT_EINVAL;
    }
    
    dev->duty_cycle = duty_cycle;
    
    /* Calculate pulse width in ticks */
    pulse_width = Chip_SCTPWM_PercentageToTicks(LPC_SCT, duty_cycle);
    
    /* Set PWM pulse */
    Chip_SCTPWM_SetPulse(LPC_SCT, dev->channel + 1, pulse_width);
    
    return RT_EOK;
}

/**
 * @brief Set PWM frequency
 * @param dev: PWM device
 * @param frequency: Frequency in Hz
 * @return RT_EOK if success
 */
rt_err_t pwm_set_frequency(struct pwm_device *dev, rt_uint32_t frequency)
{
    rt_uint32_t sct_tick;
    
    if (!dev || frequency < PWM_MIN_FREQUENCY || frequency > PWM_MAX_FREQUENCY) {
        return -RT_EINVAL;
    }
    
    dev->frequency = frequency;
    
    /* Calculate SCT tick count */
    sct_tick = Chip_Clock_GetSystemClockRate() / frequency;
    
    /* Update SCT rate */
    Chip_SCTPWM_SetRate(LPC_SCT, sct_tick);
    
    /* Restore previous duty cycle */
    return pwm_set_duty_cycle(dev, dev->duty_cycle);
}

/**
 * @brief Enable PWM output
 * @param dev: PWM device
 * @return RT_EOK if success
 */
rt_err_t pwm_enable(struct pwm_device *dev)
{
    if (!dev || dev->channel >= 6) {
        return -RT_EINVAL;
    }
    
    /* Start SCT */
    Chip_SCTPWM_Start(LPC_SCT);
    
    return RT_EOK;
}

/**
 * @brief Disable PWM output
 * @param dev: PWM device
 * @return RT_EOK if success
 */
rt_err_t pwm_disable(struct pwm_device *dev)
{
    if (!dev || dev->channel >= 6) {
        return -RT_EINVAL;
    }
    
    /* Stop SCT */
    Chip_SCTPWM_Stop(LPC_SCT);
    
    return RT_EOK;
}

/**
 * @brief Get PWM frequency
 * @param dev: PWM device
 * @return PWM frequency in Hz
 */
rt_uint32_t pwm_get_frequency(struct pwm_device *dev)
{
    if (!dev) {
        return 0;
    }
    return dev->frequency;
}

/**
 * @brief Get PWM duty cycle
 * @param dev: PWM device
 * @return Duty cycle (0-100%)
 */
rt_uint8_t pwm_get_duty_cycle(struct pwm_device *dev)
{
    if (!dev) {
        return 0;
    }
    return dev->duty_cycle;
}

/**
 * @brief Initialize PWM device
 * @param name: Device name
 * @param channel: PWM channel (0-5)
 * @param frequency: PWM frequency in Hz
 * @param duty_cycle: Initial duty cycle (0-100%)
 * @return RT_EOK if success
 */
rt_err_t rt_hw_pwm_init(const char *name, rt_uint8_t channel, 
                        rt_uint32_t frequency, rt_uint8_t duty_cycle)
{
    struct pwm_device *dev;
    rt_err_t ret;
    
    if (!name || channel >= 6 || frequency < PWM_MIN_FREQUENCY || 
        frequency > PWM_MAX_FREQUENCY || duty_cycle > 100) {
        return -RT_EINVAL;
    }
    
    /* Initialize SCT on first PWM device creation */
    if (!pwm_initialized) {
        ret = pwm_sct_init(frequency);
        if (ret != RT_EOK) {
            return ret;
        }
        pwm_initialized = RT_TRUE;
    }
    
    /* Initialize PWM pin */
    ret = pwm_pin_init(channel);
    if (ret != RT_EOK) {
        return ret;
    }
    
    /* Get PWM device */
    dev = &pwm_devices[channel];
    
    /* Initialize device structure */
    dev->parent.type = RT_Device_Class_Miscellaneous;
    dev->name = (char *)name;
    dev->channel = channel;
    dev->frequency = frequency;
    dev->duty_cycle = duty_cycle;
    
    /* Set initial duty cycle */
    ret = pwm_set_duty_cycle(dev, duty_cycle);
    if (ret != RT_EOK) {
        return ret;
    }
    
    /* Enable PWM output */
    ret = pwm_enable(dev);
    if (ret != RT_EOK) {
        return ret;
    }
    
    rt_kprintf("PWM device %s on channel %d initialized: freq=%d Hz, duty=%d%%\r\n",
               name, channel, frequency, duty_cycle);
    
    return RT_EOK;
}

/**
 * @brief RT-Thread PWM device initialization
 */
int rt_hw_pwm_device_init(void)
{
#ifdef BSP_USING_PWM
    /* Initialize PWM devices based on configuration */
    
#ifdef BSP_USING_PWM_CH0
    rt_hw_pwm_init("pwm0", PWM_CHANNEL_0, 1000, 50);
#endif
    
#ifdef BSP_USING_PWM_CH1
    rt_hw_pwm_init("pwm1", PWM_CHANNEL_1, 1000, 50);
#endif
    
#ifdef BSP_USING_PWM_CH2
    rt_hw_pwm_init("pwm2", PWM_CHANNEL_2, 1000, 50);
#endif
    
#ifdef BSP_USING_PWM_CH3
    rt_hw_pwm_init("pwm3", PWM_CHANNEL_3, 1000, 50);
#endif
    
#ifdef BSP_USING_PWM_CH4
    rt_hw_pwm_init("pwm4", PWM_CHANNEL_4, 1000, 50);
#endif
    
#ifdef BSP_USING_PWM_CH5
    rt_hw_pwm_init("pwm5", PWM_CHANNEL_5, 1000, 50);
#endif
    
#endif /* BSP_USING_PWM */
    
    return 0;
}
INIT_DEVICE_EXPORT(rt_hw_pwm_device_init);

#endif /* RT_USING_PWM */
