/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      first implementation
 * 2025-01-15     Jason        LPC824 board initialization
 * 2025-01-16     Jason        Add power management functions
 */
#include <stdint.h>
#include <rthw.h>
#include <rtthread.h>

#include "board.h"
#include "peri_driver.h"
#include "usart.h"

/* Power mode enumeration */
typedef enum {
    POWER_MODE_SLEEP = 0,       /*!< Sleep mode */
    POWER_MODE_DEEP_SLEEP = 1,  /*!< Deep sleep mode */
    POWER_MODE_POWER_DOWN = 2,  /*!< Power down mode */
    POWER_MODE_DEEP_POWERDOWN = 3  /*!< Deep power down mode */
} power_mode_t;

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
    /* Enter critical section */
    rt_enter_critical();
    
    /* Infinite loop */
    while(1)
    {
    }
}

/**
 * This is the timer interrupt service routine.
 */
void SysTick_Handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    rt_tick_increase();

    /* leave interrupt */
    rt_interrupt_leave();
}

/**
 * This function will initialize LPC8XX board.
 */
void rt_hw_board_init(void)
{
    /* Update system core clock */
    SystemCoreClockUpdate();
    
    /* Configure and start SysTick timer */
    SysTick_Config(SystemCoreClock / RT_TICK_PER_SECOND);
    
    /* Initialize GPIO port */
    Chip_GPIO_Init(LPC_GPIO_PORT);

#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#if defined(RT_USING_CONSOLE) && defined(RT_USING_DEVICE)
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif
}

/**
 * @brief Enter sleep mode
 * @note ARM Cortex-M0+ core is stopped, peripherals and memories remain active
 */
void board_enter_sleep(void)
{
    Chip_PMU_SleepState(LPC_PMU);
}

/**
 * @brief Enter deep sleep mode
 * @note Peripherals receive no internal clocks, flash in stand-by
 * Wake sources: WWDT, WKT, BOD, GPIO interrupt
 */
void board_enter_deep_sleep(void)
{
    Chip_PMU_DeepSleepState(LPC_PMU);
}

/**
 * @brief Enter power down mode
 * @note Most power consumption is saved, only GPIO interrupt can wake up
 * RAM and registers are preserved
 */
void board_enter_power_down(void)
{
    Chip_PMU_PowerDownState(LPC_PMU);
}

/**
 * @brief Enter deep power down mode
 * @note Maximum power savings, system is almost completely shut down
 * Wake sources: WAKEUP pin or self wake-up timer
 */
void board_enter_deep_powerdown(void)
{
    Chip_PMU_DeepPowerDownState(LPC_PMU);
}

/**
 * @brief Enter power mode with specified mode
 * @param mode: Power mode (0=sleep, 1=deep sleep, 2=power down, 3=deep power down)
 */
void board_enter_power_mode(power_mode_t mode)
{
    switch(mode) {
        case POWER_MODE_SLEEP:
            board_enter_sleep();
            break;
        case POWER_MODE_DEEP_SLEEP:
            board_enter_deep_sleep();
            break;
        case POWER_MODE_POWER_DOWN:
            board_enter_power_down();
            break;
        case POWER_MODE_DEEP_POWERDOWN:
            board_enter_deep_powerdown();
            break;
        default:
            board_enter_deep_sleep();
            break;
    }
}

/*@}*/
