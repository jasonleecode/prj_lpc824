/*
 * @brief I2CM bus master example reading eeprom 24C02
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2014
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

/* I2CM transfer record */
static I2CM_XFER_T  i2cmXferRec;
/* System clock is set to 24MHz, I2C clock is set to 600kHz */
#define I2C_CLK_DIVIDER         (40)
/* 100KHz I2C bit-rate */
#define I2C_BITRATE             (100000)

/* 7-bit I2C addresses of 24C02 */
/* Note: The ROM code requires the address to be between bits [6:0]
         bit 7 is ignored */
#define I2C_ADDR_EEPROM           (0x50)
#define BYTE_PER_PAGE             (8)
#define ADDRESS_RANGE             (255)
#define BUF_SIZE				(8 * 30)



/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Initializes pin muxing for I2C interface */
static void Init_I2C_PinMux(void)
{
#if (defined(BOARD_NXP_LPCXPRESSO_812) || defined(BOARD_LPC812MAX) || defined(BOARD_NXP_LPCXPRESSO_824))
	/* Enable the clock to the Switch Matrix */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);
#if defined(BOARD_NXP_LPCXPRESSO_824)
	Chip_SWM_EnableFixedPin(SWM_FIXED_I2C0_SDA);
	Chip_SWM_EnableFixedPin(SWM_FIXED_I2C0_SCL);
#else
	/* Connect the I2C_SDA and I2C_SCL signals to port pins(P0.10, P0.11) */
	Chip_SWM_MovablePinAssign(SWM_I2C_SDA_IO, 10);
	Chip_SWM_MovablePinAssign(SWM_I2C_SCL_IO, 11);
#endif

	/* Enable Fast Mode Plus for I2C pins */
	Chip_IOCON_PinSetI2CMode(LPC_IOCON, IOCON_PIO10, PIN_I2CMODE_FASTPLUS);
	Chip_IOCON_PinSetI2CMode(LPC_IOCON, IOCON_PIO11, PIN_I2CMODE_FASTPLUS);

	/* Disable the clock to the Switch Matrix to save power */
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);
#else
	/* Configure your own I2C pin muxing here if needed */
#warning "No I2C pin muxing defined"
#endif
}

/* Setup I2C handle and parameters */
static void setupI2CMaster()
{
	/* Enable I2C clock and reset I2C peripheral */
	Chip_I2C_Init(LPC_I2C);

	/* Setup clock rate for I2C */
	Chip_I2C_SetClockDiv(LPC_I2C, I2C_CLK_DIVIDER);

	/* Setup I2CM transfer rate */
	Chip_I2CM_SetBusSpeed(LPC_I2C, I2C_BITRATE);

	/* Enable Master Mode */
	Chip_I2CM_Enable(LPC_I2C);
}

/* Function to wait for I2CM transfer completion */
static void WaitForI2cXferComplete(I2CM_XFER_T *xferRecPtr)
{
	/* Test for still transferring data */
	while (xferRecPtr->status == I2CM_STATUS_BUSY) {
		/* Sleep until next interrupt */
		__WFI();
	}
}

/* Function to setup and execute I2C transfer request */
static void SetupXferRecAndExecute(uint8_t devAddr,
								   uint8_t *txBuffPtr,
								   uint16_t txSize,
								   uint8_t *rxBuffPtr,
								   uint16_t rxSize)
{
	/* Setup I2C transfer record */
	i2cmXferRec.slaveAddr = devAddr;
	i2cmXferRec.status = 0;
	i2cmXferRec.txSz = txSize;
	i2cmXferRec.rxSz = rxSize;
	i2cmXferRec.txBuff = txBuffPtr;
	i2cmXferRec.rxBuff = rxBuffPtr;

	Chip_I2CM_Xfer(LPC_I2C, &i2cmXferRec);
	/* Enable Master Interrupts */
	Chip_I2C_EnableInt(LPC_I2C, I2C_INTENSET_MSTPENDING | I2C_INTENSET_MSTRARBLOSS | I2C_INTENSET_MSTSTSTPERR);
	/* Wait for transfer completion */
	WaitForI2cXferComplete(&i2cmXferRec);
	/* Clear all Interrupts */
	Chip_I2C_ClearInt(LPC_I2C, I2C_INTENSET_MSTPENDING | I2C_INTENSET_MSTRARBLOSS | I2C_INTENSET_MSTSTSTPERR);
}


/* function to write a byte to eeprom */
static void WrEepromByte(uint8_t byteAddr, uint8_t byteData)
{
	uint8_t txBuf[2];
	
	txBuf[0] = byteAddr;			/*EEPROM byte address*/
	txBuf[1] = byteData;			/*EEPROM writen byte */
	
	SetupXferRecAndExecute(I2C_ADDR_EEPROM, txBuf, 2, NULL, 0);
}

/* function to write a page to eeprom, one page is 8 bytes*/
static void WrEepromPage(uint8_t byteAddr, uint8_t *pageData, uint8_t numCnt)
{
	uint8_t index;
	uint8_t txBuf[BYTE_PER_PAGE+1];
	
	if(numCnt > BYTE_PER_PAGE)
	{
		return;
	}

	txBuf[0] = byteAddr;
	for(index = 0; index < numCnt; index++)
	{
		txBuf[index+1] = pageData[index];
	}

	SetupXferRecAndExecute(I2C_ADDR_EEPROM, txBuf, numCnt+1, NULL, 0);
}

/* function to read a byte from eeprom */
static void RdEepromByte(uint8_t byteAddr, uint8_t *byteData)
{
	uint8_t txBuf = byteAddr;
	
	SetupXferRecAndExecute(I2C_ADDR_EEPROM, &txBuf, 1, byteData, 1);
}

/* function to read data from eeprom */
static void RdEeprom(uint8_t byteAddr, uint8_t *rdData, uint8_t numCnt)
{
	uint8_t txBuf = byteAddr;

	SetupXferRecAndExecute(I2C_ADDR_EEPROM, &txBuf, 1, rdData, numCnt);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Handle I2C interrupt by calling I2CM interrupt transfer handler
 * @return	Nothing
 */
void I2C_IRQHandler(void)
{
	/* Call I2CM ISR function with the I2C device and transfer rec */
	Chip_I2CM_XferHandler(LPC_I2C, &i2cmXferRec);
}


	
/**
 * @brief	Main routine for I2C example
 * @return	Function should not exit
 */

uint8_t rdBuf[BUF_SIZE] = {0};
uint8_t wrBuf[BUF_SIZE] = {0};
	
int main(void)
{
	uint32_t i,n;
	uint8_t testFail = 0;
	
	/* Generic Initialization */
	SystemCoreClockUpdate();

	/* Init activity LED */
	Board_Init();
	DEBUGSTR("EEPROM demo\r\n");
	DEBUGOUT("System Clock: %uMHz\r\n", SystemCoreClock / 1000000);
	DEBUGOUT("Device ID: 0x%x\r\n", Chip_SYSCTL_GetDeviceID());

	/* Setup I2C pin muxing */
	Init_I2C_PinMux();

	/* Allocate I2C handle, setup I2C rate, and initialize I2C clocking */
	setupI2CMaster();

	/* Enable the interrupt for the I2C */
	NVIC_EnableIRQ(I2C_IRQn);

	/* Init master sending buffer */
	for(i=0; i<BUF_SIZE; i++)
	{
		wrBuf[i] = i+1;
	}
	
	/* Master issues I2C writing transfer to eeprom, one byte at a time */
	for(i=0; i<BUF_SIZE/8; i++)
	{
		WrEepromPage(8*i, &wrBuf[8*i], BYTE_PER_PAGE);
		for(n=0; n<5000;n++);
	}
	
	/* Master reads back the data sent to eeprom */
	RdEeprom(0,rdBuf,BUF_SIZE);
	
	/* Verify the data sent out and read back */
	for(i=0; i<BUF_SIZE; i++)
	{
		/* flash the red led, if the verification fails */
		if(rdBuf[i] != wrBuf[i])
		{
			testFail = 1;
			Board_LED_Toggle(0);
			break;
		}	
	}
	
	/* flash the green led, if the verification succeeds */
	if(testFail == 0)
	{
		Board_LED_Toggle(1);
	}
	while(1)
	{
		__WFI();
	}
	
}
