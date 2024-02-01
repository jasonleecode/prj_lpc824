/*
 * @brief UART interrupt example with ring buffers
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

#include "chip.h"
#include "board.h"
#include "string.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* Set the default UART, IRQ number, and IRQ handler name */
LPC_USART_T *g_pUSART = LPC_USART0;
#define LPC_IRQNUM      UART0_IRQn


/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* UART Pin mux function - note that SystemInit() may already setup your
   pin muxing at system startup */
static void Init_UART_PinMux(void)
{
	/* Enable the clock to the Switch Matrix */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);

	Chip_SWM_MovablePinAssign(SWM_U0_TXD_O, 26);
	Chip_SWM_MovablePinAssign(SWM_U0_RXD_I, 16);
	Chip_IOCON_PinSetToDefault(LPC_IOCON, IOCON_PIO16);
	Chip_IOCON_PinSetToDefault(LPC_IOCON, IOCON_PIO27);

	/* Disable the clock to the Switch Matrix to save power */
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

const uint8_t *g_pcFlash = (const uint8_t*)0;
volatile uint8_t g_isClearToSend = TRUE;	// Status flag shows that app is now ready for next transmit
volatile uint8_t g_txIrqPacketFlag;	// A packet has been sent flag.

#define DUMP_RANGE	0x2000
#define APP_PACKET_SIZE 1024	
uint8_t g_rxBuf[APP_PACKET_SIZE];
volatile uint32_t g_txNdx;
volatile uint32_t g_rxNdx;			// Current array index for g_rxBuf[]
volatile uint8_t g_rxErrorCode;
volatile uint8_t g_rxPacketFlag;	// A packet has been received flag.

volatile uint32_t g_tick;
volatile uint32_t g_txT0;

void SysTick_Handler(void)
{
	g_tick++;
}

void DumpFlashIrq(void)
{
	if (!g_isClearToSend)
		return;

	DEBUGOUT("Dumping address 0x%04x...\n", g_pcFlash);
	g_isClearToSend = FALSE , g_txNdx = 0 , g_rxNdx = 0 , g_rxErrorCode = 0;
	memset(g_rxBuf, 0, sizeof(g_rxBuf));
	
	Chip_UART_ClearStatus(LPC_USART0, UART_STAT_OVERRUNINT|UART_STAT_DELTARXBRK|
		UART_STAT_FRM_ERRINT|UART_STAT_PAR_ERRINT|UART_STAT_RXNOISEINT);
	g_txT0 = g_tick;
	Chip_UART_SendByte(g_pUSART, g_pcFlash[g_txNdx++]);
	Chip_UART_IntEnable(g_pUSART, UART_INTEN_TXRDY);
	// Chip_UART_IntEnable(g_pUSART, UART_INTEN_TXIDLE);	
}

void UART0_IRQHandler(void)
{	
	// check if we can send new data and/or received a new byte
	uint32_t status;
	Board_LED_Set(2, 1);
	status = Chip_UART_GetStatus(g_pUSART);
	// Since more than one reason can enter this ISR, we must check
	// TX only if we still have data to send		
	if (g_txNdx < APP_PACKET_SIZE)
	{
		if (status & UART_STAT_TXRDY)
		{
			Chip_UART_SendByte(g_pUSART, g_pcFlash[g_txNdx]);
			if (++g_txNdx == APP_PACKET_SIZE) 
			{
				// all data is sent for current packet
				g_txIrqPacketFlag = 1;
				// disable TX ready/idle IRQ so we pause sending
				Chip_UART_IntDisable(g_pUSART, UART_INTEN_TXRDY | UART_INTEN_TXIDLE);
				g_pcFlash += APP_PACKET_SIZE;
				if (g_pcFlash >= (void*)DUMP_RANGE)
					g_pcFlash = 0;						
			}
		}
	}	
	
	if (status & UART_STAT_RXRDY)
	{
		if (g_rxNdx < sizeof(g_rxBuf))
			g_rxBuf[g_rxNdx] = Chip_UART_ReadByte(g_pUSART);
		else
			g_rxErrorCode = 3;	// buffer overrun
		// RXRDY flag is h/w clear when data is read
		if (++g_rxNdx == APP_PACKET_SIZE)
		{
			const uint8_t *pcFlash = (g_pcFlash == (void*)0 ? (void*)DUMP_RANGE : g_pcFlash);
			g_rxPacketFlag = 1;
			if (memcmp(pcFlash - APP_PACKET_SIZE, g_rxBuf, APP_PACKET_SIZE) != 0)
				g_rxErrorCode = 1;	// some data error
			else
			{
				// note: printing here may cause race condition: ISR and background compete debug UART
				// a good showcase of race condition
				// printf(" In ISR:Received data all OK ^_^\n");
			}
		}		
	}
	
	if (status & UART_STAT_OVERRUNINT)
	{
		Chip_UART_ClearStatus(g_pUSART, UART_STAT_OVERRUNINT);
		g_rxErrorCode = 2;	// USART h/w overrun
	}
	Board_LED_Set(2, 0);
}

/**
 * @brief	Main UART program body
 * @return	Always returns 1
 */
int main(void)
{
	SystemCoreClockUpdate();
	Board_Init();
	Init_UART_PinMux();
	Board_LED_Set(0, false);
	SysTick_Config(SystemCoreClock / 10000);

	DEBUGSTR("UART IRQ example for both TX and RX\n");
	DEBUGSTR("Data is in loopback mode so no fly wiring is required\n");
	// >>> Setup UART
	Chip_UART_Init(g_pUSART);
	Chip_UART_ConfigData(g_pUSART, UART_CFG_DATALEN_8 | UART_CFG_PARITY_NONE | UART_CFG_STOPLEN_1);
	// Assume current base clock settings are OK
	// Chip_Clock_SetUSARTNBaseClockRate((115200 * 6 * 16), true);
	Chip_UART_SetBaud(g_pUSART, 19200);
	Chip_UART_Enable(g_pUSART);
	Chip_UART_TXEnable(g_pUSART);
	
	g_pUSART->CFG |= 1UL<<15;	// loopback mode

	// we must NOT enable TX ready/idle IRQ before we want to write data
	// otherwise the IRQs will happen as soon as Uart IRQ is enabled in NVIC
	Chip_UART_IntDisable(g_pUSART, UART_INTEN_TXRDY | UART_INTEN_TXIDLE);
	
	Chip_UART_IntEnable(g_pUSART, UART_INTEN_RXRDY | UART_INTEN_OVERRUN);
	NVIC_EnableIRQ(UART0_IRQn);
	// <<<

	while(1)
	{
		if (Board_Key_GetKeyDown(0))
		{
			DumpFlashIrq();
		}
		if (g_txIrqPacketFlag == TRUE)
		{
			printf("IRQ xfer done! IRQ TX time = %d\n", g_tick - g_txT0);
			g_txIrqPacketFlag = 0;
		}
		if (g_rxPacketFlag)
		{
			if (g_rxErrorCode)
			{
				printf("Error occured during receiving :(\n");
				if (g_rxErrorCode == 2)
					printf("RX overrun occured!\n");
			}
			else
				 printf("Received data all OK ^_^\n");
			g_rxPacketFlag = 0;
			g_isClearToSend = 1;
		}
			
	}
}

