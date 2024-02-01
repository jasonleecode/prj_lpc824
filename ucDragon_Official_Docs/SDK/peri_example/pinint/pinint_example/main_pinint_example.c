/*
 * @brief Pin Interrupt example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "board.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* Board specific definitions */
#if defined(BOARD_NXP_LPCXPRESSO_812)
/* Use same pin connected to potentiometer (PIO0) */
#define PININT_KEYSW1         0

#elif defined(BOARD_LPC812MAX)
#define PININT_KEYSW1         5

#elif defined(BOARD_NXP_LPCXPRESSO_824)
#define PININT_KEYS2       12
#define PININT_KEYS3			 4
#define PININT_KEYS4			 1
#else
#error "No GPIO assigned for this board"
#endif

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/
/*********************************************************************************************************
** Function name:       myDelay
** Descriptions:        软件延时
** input parameters:    ulTime:延时时间(ms)
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void myDelay (uint32_t ulTime)
{
    uint32_t i;
    
    while (ulTime--) {
        for (i = 0; i < 2401; i++);
    }
}

volatile uint8_t g_pinIntFlags;
void PININT7_IRQHandler(void)
{
	Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH7);
	g_pinIntFlags |= 1<<0;
	Board_LED_Set(1, 1);
}

void PININT2_IRQHandler(void)
{
	Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH2);
	g_pinIntFlags |= 1<<1;
	Board_LED_Set(2, 1);
}

void SysTick_Handler(void)
{
	if (g_pinIntFlags & 1<<0) {
		Board_LED_Set(1, 0);
		g_pinIntFlags &= ~(1<<0);
	}
	
	if (g_pinIntFlags & (1<<1)) {
		Board_LED_Set(2, 0);
		g_pinIntFlags &= ~(1<<1);
	}	
		Board_LED_Toggle(0);
}

void PinINTConfig(void)
{
	Chip_PININT_Init(LPC_PININT);
	// >>> Patch PININT_KEYSW1 to PININT7, falling edge
	Chip_SYSCTL_SetPinInterrupt(7, PININT_KEYS2);
	// /* Configure channel 7 as wake up interrupt in SysCon block */
	Chip_SYSCTL_EnablePINTWakeup(7);
	/* Configure GPIO pin as input pin */
	Chip_GPIO_PinSetDIR(LPC_GPIO_PORT, 0, PININT_KEYS2, 0);

	/* Configure channel 7 interrupt as edge sensitive and falling edge interrupt */
	Chip_PININT_SetPinModeEdge(LPC_PININT, PININTCH7);
	Chip_PININT_EnableIntLow(LPC_PININT, PININTCH7);
	/* Enable interrupt in the NVIC */
	NVIC_EnableIRQ(PININT7_IRQn);
	// <<<
	
	// >>> Patch PININT_KEYSW2 to PININT2上，Low level
	Chip_SYSCTL_SetPinInterrupt(2, PININT_KEYS3);
	// /* Configure channel 2 as wake up interrupt in SysCon block */
	Chip_SYSCTL_EnablePINTWakeup(2);
	/* Configure GPIO pin as input pin */
	Chip_GPIO_PinSetDIR(LPC_GPIO_PORT, 0, PININT_KEYS3, 0);
	Chip_IOCON_PinSetMode(LPC_IOCON, IOCON_PIO12, PIN_MODE_PULLUP);

	/* Configure channel 2 interrupt as level sensitive and falling edge interrupt */
	Chip_PININT_SetPinModeLevel(LPC_PININT, PININTCH2);
	Chip_PININT_EnableIntLow(LPC_PININT, PININTCH2);
	/* Enable interrupt in the NVIC */
	NVIC_EnableIRQ(PININT2_IRQn);
	// <<<	
}
/**
 * @brief	Main program body
 * @return	Does not return
 */
int main(void) {
	// >>> Generic Initialization
	// SystemCoreClockUpdate() modifies global, must be invoked after main()
	SystemCoreClockUpdate();
	Board_Init();
	Board_LED_Set(3, true);
	Board_LED_Set(4, true);
	Board_LED_Set(5, true);
	Board_LED_Set(6, true);
	Board_LED_Set(7, true);
	/* Enable SysTick Timer */
	SysTick_Config(SystemCoreClock / 20);
	// <<<
	DEBUGOUT("LPC82x PININT example\r\nShows edge and level IRQ\r\nNote: Level IRQ needs to disable for a while to avoid imeediate re-enter\r\nHardware automatically set/clear level IRQ flags\r\n");
	PinINTConfig();

	/* Enable all clocks, even those turned off at wake power up */
	Chip_SYSCTL_SetWakeup(~(SYSCTL_SLPWAKE_IRCOUT_PD | SYSCTL_SLPWAKE_IRC_PD |
							SYSCTL_SLPWAKE_FLASH_PD | SYSCTL_SLPWAKE_SYSOSC_PD | SYSCTL_SLPWAKE_SYSOSC_PD |
							SYSCTL_SLPWAKE_SYSPLL_PD));

	/* Go to sleep mode - LED will toggle on each wakeup event */
	while (1) {
		Chip_PMU_SleepState(LPC_PMU);
		//// If DeepSleep or PowerDown is used, then SysTick timer can't work due to main clock is shut down
		//// as a result, SysTick ISR won't be entered.
		// Chip_PMU_DeepSleepState(LPC_PMU);
		// Chip_PMU_PowerDownState(LPC_PMU);
	}
}
