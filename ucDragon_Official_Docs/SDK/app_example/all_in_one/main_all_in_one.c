/*
 * @brief Blinky example using SysTick and interrupt
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
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
#include <stdio.h>
#include "board.h"
#include "w25x32.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
/***************ADC**********************************************************/
static volatile int ticks;
static bool sequenceComplete, thresholdCrossed;

#define TICKRATE_HZ (100)	/* 100 ticks per second */

#define BOARD_ADC_CH 3
/***************ADC END******************************************************/

/**************SPI***********************************************************/
#define BUFFER_SIZE         10
#define SPI_MODE_TEST       (SPI_MODE_MASTER)

#define LPC_SPI           LPC_SPI1
#define SPI_IRQ           SPI1_IRQn
#define SPIIRQHANDLER     SPI1_IRQHandler

/* Tx buffer */
static uint8_t WrBuf[BUFFER_SIZE];

/* Rx buffer */
static uint8_t RdBuf[BUFFER_SIZE];

// static SPI_CONFIG_T ConfigStruct;
static SPI_DELAY_CONFIG_T DelayConfigStruct;

/***************SPI END*******************************************************/

/***************EEPROM********************************************************/
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
/***************EEPROM end***************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

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
 
 
void myDelay(uint16_t uiTime)
{
    uint16_t i,j;
    for(i = 0; i < uiTime; i++) {
        for(j = 0; j < 5000; j++);
    }
}
/**
 * @brief	Handle interrupt from SysTick timer
 * @return	Nothing
 */
void SysTick_Handler(void)
{
	static uint32_t count;
	
	/* Every 1/2 second */
	if (count++ == TICKRATE_HZ * 2) {
		count = 0;

		/* Manual start for ADC conversion sequence A */
		Chip_ADC_StartSequencer(LPC_ADC, ADC_SEQA_IDX);
		Board_LED_Toggle(0);
	}
}

/**
 * @brief	Handle interrupt from ADC sequencer A
 * @return	Nothing
 */
void ADC_SEQA_IRQHandler(void)
{
	uint32_t pending;

	/* Get pending interrupts */
	pending = Chip_ADC_GetFlags(LPC_ADC);

	/* Sequence A completion interrupt */
	if (pending & ADC_FLAGS_SEQA_INT_MASK) {
		sequenceComplete = true;
	}

	/* Threshold crossing interrupt on ADC input channel */
	if (pending & ADC_FLAGS_THCMP_MASK(BOARD_ADC_CH)) {
		thresholdCrossed = true;
	}

	/* Clear any pending interrupts */
	Chip_ADC_ClearFlags(LPC_ADC, pending);
}

/* Initialize buffer */
static void bufferInit(void)
{
	uint16_t i;
	uint8_t ch = 0;

	for (i = 0; i < BUFFER_SIZE; i++) {
		WrBuf[i] = ch++;
	}
}


/* Verify buffer after transfer */
static uint8_t bufferVerify(void)
{
	uint16_t i;
	uint8_t *src_addr = (uint8_t *) &WrBuf[0];
	uint8_t *dest_addr = (uint8_t *) &RdBuf[0];

	for ( i = 0; i < BUFFER_SIZE; i++ ) {

		if (*src_addr != *dest_addr) {
			return 1;
		}
		src_addr++;
		dest_addr++;
	}
	return 0;
}
void SPI_Flash_Test(void)
{
/*
	   ConfigStruct.Mode = SPI_MODE_TEST;
	   ConfigStruct.ClkDiv = Chip_SPI_CalClkRateDivider(LPC_SPI, 100000);
	   ConfigStruct.ClockMode = SPI_CLOCK_MODE3;
	   ConfigStruct.DataOrder = SPI_DATA_MSB_FIRST;
	   ConfigStruct.SSELPol = SPI_SSEL_ACTIVE_LO;
*/
	DEBUGSTR("\nSPI Test start\n\r");
	Chip_SPI_Init(LPC_SPI);
	Chip_SPI_ConfigureSPI(LPC_SPI, SPI_CFG_SPI_EN|  SPI_MODE_TEST |	/* Enable master */
						  SPI_CLOCK_MODE3 |	/* Set Clock polarity to 1 */
						  SPI_CFG_MSB_FIRST_EN |/* Enable MSB first option */
						  SPI_CFG_SPOL_LO);	/* Chipselect is active low */
	LPC_SPI->TXCTRL = (0 << 16) |                                      /* 从机选择                     */
                    (0 << 20) |                                      /* 传输不结束                   */
                    (1 << 21) |                                      /* 帧结束                       */
                    (0 << 22) |                                      /* 不忽略接收                   */
                    (7 << 24);                                       /* 帧长度：8位                  */
    
	
	DEBUGOUT("Start to Read ID...\n");                                        
	DEBUGOUT("Device ID: 0X%X6\n\r", flash_read_id(Jedec_ID)&0x00FFFFFF); //读ID
	DEBUGOUT("Start to erase sector 0...\n\r");
	flash_sector_erase(0x000000);
  myDelay(300); 		                                                  /* 延时                         */
	bufferInit();
  DEBUGOUT("Start to write sector 0...\n\r");
  flash_write_sector(0x000000, WrBuf, BUFFER_SIZE);                          /* 以0x0为起始地址，将WrBuf  */
  myDelay(30);
    
	DEBUGOUT("Start to read sector 0...\n\r");                                                                    /* 数组里的20个数据写入芯片     */
	flash_read_data(0x000000,  RdBuf,BUFFER_SIZE);                            /* 以0x0为起始地址，读20个   */
     
	if(bufferVerify()==0)
	{                                                   
		DEBUGOUT("Verify ok...\n");
		DEBUGSTR("SPI Test OK!\r\n");
	}
}

uint8_t rdBuf[BUF_SIZE] = {0};
uint8_t wrBuf[BUF_SIZE] = {0};
void EEPROM_Test(void)
{
	uint32_t i,n;
	uint8_t testFail = 0;
	DEBUGSTR("\nEEPROM Test start\n\r");
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
			DEBUGOUT("Verify Fail...\n\r");
			DEBUGSTR("EEPROM Test Fail\n\r");
			break;
		}	
	}
	
	/* flash the green led, if the verification succeeds */
	if(testFail == 0)
	{
		DEBUGOUT("Verify ok...\n\r");
		DEBUGSTR("EEPROM Test OK!\n\r");
	}

}
	
void ADC_Test(void)
{
	uint32_t rawSample;
	int j;
	DEBUGSTR("\nADC Test start\n\r");
		/* Setup ADC for 12-bit mode and normal power */
	Chip_ADC_Init(LPC_ADC, 0);

	/* Need to do a calibration after initialization and trim */
	Chip_ADC_StartCalibration(LPC_ADC);
	while (!(Chip_ADC_IsCalibrationDone(LPC_ADC))) {}

	/* Setup for maximum ADC clock rate using sycnchronous clocking */
	Chip_ADC_SetClockRate(LPC_ADC, ADC_MAX_SAMPLE_RATE);

	/* Optionally, you can setup the ADC to use asycnchronous clocking mode.
	   To enable this, mode use 'LPC_ADC->CTRL |= ADC_CR_ASYNMODE;'.
	   In asycnchronous clocking mode mode, the following functions are
	   used to set and determine ADC rates:
	   Chip_Clock_SetADCASYNCSource();
	   Chip_Clock_SetADCASYNCClockDiv();
	   Chip_Clock_GetADCASYNCRate();
	   clkRate = Chip_Clock_GetADCASYNCRate() / Chip_Clock_GetADCASYNCClockDiv; */

	/* Setup sequencer A for ADC channel 3, EOS interrupt */
	/* Setup a sequencer to do the following:
	   Perform ADC conversion of ADC channels 3 only */
	Chip_ADC_SetupSequencer(LPC_ADC, ADC_SEQA_IDX,
							(ADC_SEQ_CTRL_CHANSEL(BOARD_ADC_CH) | ADC_SEQ_CTRL_MODE_EOS));

	/* Enable the clock to the Switch Matrix */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);

	/* Configure the SWM for P0-23 as the input for the ADC1 */
	Chip_SWM_EnableFixedPin(SWM_FIXED_ADC3);

	/* Disable the clock to the Switch Matrix to save power */
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);


	/* Setup threshold 0 low and high values to about 25% and 75% of max */
	Chip_ADC_SetThrLowValue(LPC_ADC, 0, ((1 * 0xFFF) / 4));
	Chip_ADC_SetThrHighValue(LPC_ADC, 0, ((3 * 0xFFF) / 4));

	/* Clear all pending interrupts */
	Chip_ADC_ClearFlags(LPC_ADC, Chip_ADC_GetFlags(LPC_ADC));

	/* Enable ADC overrun and sequence A completion interrupts */
	Chip_ADC_EnableInt(LPC_ADC, (ADC_INTEN_SEQA_ENABLE | ADC_INTEN_OVRRUN_ENABLE));

	/* Use threshold 0 for ADC channel and enable threshold interrupt mode for
	   channel as crossing */
	Chip_ADC_SelectTH0Channels(LPC_ADC, ADC_THRSEL_CHAN_SEL_THR1(BOARD_ADC_CH));
	Chip_ADC_SetThresholdInt(LPC_ADC, BOARD_ADC_CH, ADC_INTEN_THCMP_CROSSING);

	/* Enable ADC NVIC interrupt */
	NVIC_EnableIRQ(ADC_SEQA_IRQn);

	/* Enable sequencer */
	Chip_ADC_EnableSequencer(LPC_ADC, ADC_SEQA_IDX);

	/* This example uses the periodic sysTick to manually trigger the ADC,
	   but a periodic timer can be used in a match configuration to start
	   an ADC sequence without software intervention. */
	SysTick_Config(SystemCoreClock / TICKRATE_HZ);

	/* Loop forever */
	while (1) {
		__WFI();
		
			if (thresholdCrossed) {
			thresholdCrossed = false;
			DEBUGSTR("********ADC threshold event********\r\n");
		}

		/* Is a conversion sequence complete? */
		if (sequenceComplete) {
			sequenceComplete = false;

			/* Get raw sample data for channels 0-11 */
			for (j = 0; j < 12; j++) {
				rawSample = Chip_ADC_GetDataReg(LPC_ADC, j);

				/* Show some ADC data */
				if (rawSample & (ADC_DR_OVERRUN | ADC_SEQ_GDAT_DATAVALID)) {
					DEBUGOUT("Chan: %d Val: %d\r\n", j, ADC_DR_RESULT(rawSample));
					DEBUGOUT("Threshold range: 0x%x ", ADC_DR_THCMPRANGE(rawSample));
					DEBUGOUT("Threshold cross: 0x%x\r\n", ADC_DR_THCMPCROSS(rawSample));
					DEBUGOUT("Overrun: %s \r\n", (rawSample & ADC_DR_OVERRUN) ? "true" : "false");
					DEBUGOUT("Data Valid: %s\r\n\r\n", (rawSample & ADC_SEQ_GDAT_DATAVALID) ? "true" : "false");
				}
			}
		}
	}

}
void LED_Test(void)
{
	uint8_t i;
	DEBUGSTR("\nLED Test start\r\n");
	for(i=0; i<BOARD_LED_CNT; i++)
	{
		Board_LED_Set(i,true);
		myDelay(50);
		Board_LED_Set(i,false);
		myDelay(50);
	}
}
/**
 * @brief	main routine for blinky example
 * @return	Function should not exit.
 */
int main(void)
{
	
	// SystemCoreClockUpdate会更新全局变量的值，因此只能在进入main()后再调用
	SystemCoreClockUpdate();
	Board_Init();
	DEBUGSTR("ADC sequencer demo\r\n");
	DEBUGOUT("System Clock: %uMHz\r\n", SystemCoreClock / 1000000);
	DEBUGOUT("Device ID: 0x%x\r\n", Chip_SYSCTL_GetDeviceID());
	
	LED_Test();
	SPI_Flash_Test();
	EEPROM_Test();
	ADC_Test();
	
}
