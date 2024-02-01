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

#define BUF_SIZE				(8 * 50)

/* System clock is set to 24MHz, I2C clock is set to 600kHz */
#define I2C_CLK_DIVIDER         (40)
/* 100KHz I2C bit-rate */
#define I2C_BITRATE             (100000)


/** Our slave address and I2C information */
#define LPC_I2C_SLAVE         LPC_I2C1
#define LPC_I2C_SLAVE_HANDLER      I2C1_IRQHandler
#define LPC_I2C_SLAVE_IRQNUM           I2C1_IRQn
#define I2C_ADDR_SLAVE          (0x50)


/* I2CM transfer record */
static I2CM_XFER_T  i2cmXferRec;

uint8_t slaveBuf[BUF_SIZE] = {0};
uint8_t masterSendBuf[BUF_SIZE] = {0};
uint8_t masterRecvBuf[BUF_SIZE] = {0};

STATIC RINGBUFF_T i2cring;	


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
	
	Chip_SWM_MovablePinAssign(SWM_I2C1_SDA_IO, 13);
	Chip_SWM_MovablePinAssign(SWM_I2C1_SCL_IO, 17);
	
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

static void setupI2CSlave(void)
{
	/* Some common I2C init was performed in setupI2CMaster(), so it doesn't
	   need to be done again for the slave setup. */

	Chip_I2C_Init(LPC_I2C_SLAVE);
	/* Emulated EEPROM 0 is on slave index 0 */
	Chip_I2CS_SetSlaveAddr(LPC_I2C_SLAVE, 0, I2C_ADDR_SLAVE);
	/* Disable Qualifier for Slave Address 0 */
	Chip_I2CS_SetSlaveQual0(LPC_I2C_SLAVE, false, 0);
	/* Enable Slave Address 0 */
	Chip_I2CS_EnableSlaveAddr(LPC_I2C_SLAVE, 0);

	/* Clear interrupt status and enable slave interrupts */
	Chip_I2CS_ClearStatus(LPC_I2C_SLAVE, I2C_STAT_SLVDESEL);
	Chip_I2C_EnableInt(LPC_I2C_SLAVE, I2C_INTENSET_SLVPENDING | I2C_INTENSET_SLVDESEL);

	/* Enable I2C slave interface */
	Chip_I2CS_Enable(LPC_I2C_SLAVE);
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

/* Handler for slave start callback */
static void processSlaveTransferStart(uint8_t addr)
{
	
}

/* Handler for slave send callback */
static uint8_t processSlaveTransferSend(uint8_t *data)
{
	/* Send data from emulated EEPROM */
	RingBuf_Read1Byte(&i2cring, data);

	return 0;
}

/* Handler for slave receive callback */
static uint8_t processSlaveTransferRecv(uint8_t data)
{
	RingBuf_Write1Byte(&i2cring, &data);
	
	return 0;
}

/* Handler for slave transfer complete callback */
static void processSlaveTransferDone(void)
{
	/* Nothing needs to be done here */
}

/* I2C slavecallback function list */
const static I2CS_XFER_T i2csCallBacks = {
	&processSlaveTransferStart,
	&processSlaveTransferSend,
	&processSlaveTransferRecv,
	&processSlaveTransferDone
};

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
 * @brief	Handle I2C1 interrupt by calling I2CS interrupt transfer handler
 * @return	Nothing
 */

void LPC_I2C_SLAVE_HANDLER(void)
{
	uint32_t state = Chip_I2C_GetPendingInt(LPC_I2C_SLAVE);

	/* I2C slave related interrupt */
	while (state & (I2C_INTENSET_SLVPENDING | I2C_INTENSET_SLVDESEL)) {
		Chip_I2CS_XferHandler(LPC_I2C_SLAVE, &i2csCallBacks);

		/* Update state */
		state = Chip_I2C_GetPendingInt(LPC_I2C_SLAVE);
	}
}

	
/**
 * @brief	Main routine for I2C example
 * @return	Function should not exit
 */

int main(void)
{
	uint32_t i;
	uint8_t testFail = 0;
	
	/* Generic Initialization */
	SystemCoreClockUpdate();

	/* Init Ring Buffer for I2C Slave transfer*/
	RingBuf_Init(&i2cring, slaveBuf, BUF_SIZE);

	/* Init activity LED */
	Chip_GPIO_PinSetDIR(LPC_GPIO_PORT, 0, 12, 1);
	Chip_GPIO_PinSetState(LPC_GPIO_PORT, 0, 12, 1);
	Chip_GPIO_PinSetDIR(LPC_GPIO_PORT, 0, 16, 1);
	Chip_GPIO_PinSetState(LPC_GPIO_PORT, 0, 16, 1);

	/* Setup I2C pin muxing */
	Init_I2C_PinMux();

	/* Allocate I2C handle, setup I2C rate, and initialize I2C clocking */
	setupI2CMaster();
	/*Set slave address and enable slave function */
	setupI2CSlave();

	/* Enable the interrupt for the I2C */
	NVIC_EnableIRQ(I2C_IRQn);
	NVIC_EnableIRQ(LPC_I2C_SLAVE_IRQNUM);
	/* Set up I2C intertupt priority, slave function should have higher priority */
	NVIC_SetPriority(I2C_IRQn,3);
	NVIC_SetPriority(LPC_I2C_SLAVE_IRQNUM,1);

	/* Init master transfer buffer */
	for(i=0; i<BUF_SIZE; i++)
	{
		masterSendBuf[i] = i;
	}
	/* Issue I2C transfer, master firstly write to slave, then read back */
	SetupXferRecAndExecute(I2C_ADDR_SLAVE, masterSendBuf, BUF_SIZE, NULL, 0);
	for(i=0; i<100; i++);
	SetupXferRecAndExecute(I2C_ADDR_SLAVE, NULL, 0, masterRecvBuf, BUF_SIZE);
	
	/* Verify the data sent out and read back, it should be the same */
	/* If verification fails, flash the red led */
	for(i=0; i<BUF_SIZE; i++)
	{
		if(masterRecvBuf[i] != masterSendBuf[i])
		{
			testFail = 1;
			Board_LED_Toggle(0);
			break;
		}
		
	}
	/* If verification succeeds, flash green led */
	if(testFail == 0)
	{
		Board_LED_Toggle(1);
	}
	while(1)
	{
		__WFI();
	}
	
}
