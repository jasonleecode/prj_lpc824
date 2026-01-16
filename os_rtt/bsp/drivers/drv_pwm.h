/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-01-16     Jason        LPC824 PWM driver header
 */

#ifndef __DRV_PWM_H__
#define __DRV_PWM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <rtthread.h>

/* PWM device structure */
struct pwm_device {
    struct rt_device parent;
    char *name;
    rt_uint32_t frequency;      /* PWM frequency in Hz */
    rt_uint32_t pulse_width;    /* Pulse width in microseconds */
    rt_uint8_t channel;         /* PWM channel (0-5) */
    rt_uint8_t duty_cycle;      /* Duty cycle 0-100% */
};

/* PWM configuration structure */
typedef struct {
    rt_uint32_t frequency;      /* PWM frequency in Hz (1-10000) */
    rt_uint8_t duty_cycle;      /* Duty cycle 0-100% */
    rt_uint8_t channel;         /* PWM channel 0-5 */
} pwm_config_t;

/**
 * @brief Initialize PWM device
 * @param name: Device name
 * @param channel: PWM channel (0-5)
 * @param frequency: PWM frequency in Hz
 * @param duty_cycle: Initial duty cycle (0-100%)
 * @return RT_EOK if success
 */
rt_err_t rt_hw_pwm_init(const char *name, rt_uint8_t channel, 
                        rt_uint32_t frequency, rt_uint8_t duty_cycle);

/**
 * @brief Set PWM frequency
 * @param dev: PWM device
 * @param frequency: Frequency in Hz
 * @return RT_EOK if success
 */
rt_err_t pwm_set_frequency(struct pwm_device *dev, rt_uint32_t frequency);

/**
 * @brief Set PWM duty cycle
 * @param dev: PWM device
 * @param duty_cycle: Duty cycle (0-100%)
 * @return RT_EOK if success
 */
rt_err_t pwm_set_duty_cycle(struct pwm_device *dev, rt_uint8_t duty_cycle);

/**
 * @brief Enable PWM output
 * @param dev: PWM device
 * @return RT_EOK if success
 */
rt_err_t pwm_enable(struct pwm_device *dev);

/**
 * @brief Disable PWM output
 * @param dev: PWM device
 * @return RT_EOK if success
 */
rt_err_t pwm_disable(struct pwm_device *dev);

/**
 * @brief Get PWM frequency
 * @param dev: PWM device
 * @return PWM frequency in Hz
 */
rt_uint32_t pwm_get_frequency(struct pwm_device *dev);

/**
 * @brief Get PWM duty cycle
 * @param dev: PWM device
 * @return Duty cycle (0-100%)
 */
rt_uint8_t pwm_get_duty_cycle(struct pwm_device *dev);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_PWM_H__ */
