/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-09-22     Bernard      add board.h to this bsp
 * 2025-01-16     Jason        Add power management interfaces
 */

// <<< Use Configuration Wizard in Context Menu >>>
#ifndef __BOARD_H__
#define __BOARD_H__


#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN    (&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="HEAP"
#define HEAP_BEGIN    (__segment_end("HEAP"))
#else
extern int __bss_end;
#define HEAP_BEGIN    (&__bss_end)
#endif

#define LPC824_SRAM_SIZE         8
#define LPC824_SRAM_END         (0x10000000 + LPC824_SRAM_SIZE * 1024)

#define HEAP_END          LPC824_SRAM_END

/* LED definitions */
#define LED0_PORT         0
#define LED0_PIN          7
#define LED0_IO           (LPC_GPIO_PORT_PIN(LED0_PORT, LED0_PIN))

#define LED1_PORT         0
#define LED1_PIN          13
#define LED1_IO           (LPC_GPIO_PORT_PIN(LED1_PORT, LED1_PIN))

#define LED2_PORT         0
#define LED2_PIN          16
#define LED2_IO           (LPC_GPIO_PORT_PIN(LED2_PORT, LED2_PIN))

#define LED3_PORT         0
#define LED3_PIN          17
#define LED3_IO           (LPC_GPIO_PORT_PIN(LED3_PORT, LED3_PIN))

#define LED4_PORT         0
#define LED4_PIN          19
#define LED4_IO           (LPC_GPIO_PORT_PIN(LED4_PORT, LED4_PIN))

#define LED5_PORT         0
#define LED5_PIN          27
#define LED5_IO           (LPC_GPIO_PORT_PIN(LED5_PORT, LED5_PIN))

#define LED6_PORT         0
#define LED6_PIN          28
#define LED6_IO           (LPC_GPIO_PORT_PIN(LED6_PORT, LED6_PIN))

#define LED7_PORT         0
#define LED7_PIN          18
#define LED7_IO           (LPC_GPIO_PORT_PIN(LED7_PORT, LED7_PIN))

void rt_hw_board_init(void);

void Error_Handler(void);

/* Power management interfaces */
void board_enter_sleep(void);
void board_enter_deep_sleep(void);
void board_enter_power_down(void);
void board_enter_deep_powerdown(void);

#endif

//*** <<< end of configuration section >>>    ***
