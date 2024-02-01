/*
 * @brief SPI example
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
#include "w25x32.h"
/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/


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

static SPI_DATA_SETUP_T XfSetup;
static volatile uint8_t  isXferCompleted = 0;



uint32_t    GulChipID     = 0;


/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
void myDelay(uint16_t uiTime)
{
    uint16_t i,j;
    for(i = 0; i < uiTime; i++) {
        for(j = 0; j < 5000; j++);
    }
}
/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Initializes pin muxing for SPI1 interface - note that SystemInit() may
   already setup your pin muxing at system startup */
static void Init_SPI_PinMux(void)
{

	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);
	Chip_SWM_MovablePinAssign(SWM_SPI1_SSEL0_IO, 15);
	Chip_SWM_MovablePinAssign(SWM_SPI1_SCK_IO, 24);
	Chip_SWM_MovablePinAssign(SWM_SPI1_MISO_IO, 25);
	Chip_SWM_MovablePinAssign(SWM_SPI1_MOSI_IO, 26);
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);

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



/*****************************************************************************
 * Public functions
 ****************************************************************************/

#if INTERRUPT_MODE
/**
 * @brief	SPI interrupt handler sub-routine
 * @return	Nothing
 */
void SPIIRQHANDLER(void)
{
	Chip_SPI_Int_Cmd(LPC_SPI, SPI_INTENCLR_TXDYEN | SPI_INTENCLR_RXDYEN
					 | SPI_INTENCLR_RXOVEN | SPI_INTENCLR_TXUREN, DISABLE);						/* Disable all interrupt */
	if (((XfSetup.pRx) && (XfSetup.RxCnt < XfSetup.Length))
		|| ((XfSetup.pTx) && (XfSetup.TxCnt < XfSetup.Length))) {
		Chip_SPI_Int_RWFrames(LPC_SPI, &XfSetup);
		Chip_SPI_Int_Cmd(LPC_SPI, SPI_INTENSET_TXDYEN | SPI_INTENSET_RXDYEN
						 | SPI_INTENSET_RXOVEN | SPI_INTENSET_TXUREN, ENABLE);
	}
	else {
		isXferCompleted = 1;
	}
}

#endif /*INTERRUPT_MODE*/

/**
 * @brief	Main routine for SPI example
 * @return	Nothing
 */
int main(void)
{
	int i;
	SystemCoreClockUpdate();
	Board_Init();
	DEBUGSTR("SPI Flash demo\r\n");
	DEBUGOUT("System Clock: %uMHz\r\n", SystemCoreClock / 1000000);
	DEBUGOUT("Device ID: 0x%x\r\n", Chip_SYSCTL_GetDeviceID());
	/* SPI initialization */
	Init_SPI_PinMux();

	/*
	   ConfigStruct.Mode = SPI_MODE_TEST;
	   ConfigStruct.ClkDiv = Chip_SPI_CalClkRateDivider(LPC_SPI, 100000);
	   ConfigStruct.ClockMode = SPI_CLOCK_MODE3;
	   ConfigStruct.DataOrder = SPI_DATA_MSB_FIRST;
	   ConfigStruct.SSELPol = SPI_SSEL_ACTIVE_LO;
	 */
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
    
	
	DEBUGOUT("Start to Read ID...\n\r");                                        
	DEBUGOUT("Device ID: 0X%X6\r\n", flash_read_id(Jedec_ID)&0x00FFFFFF); //读ID
	DEBUGOUT("Start to erase sector 0...\n\r");
	flash_sector_erase(0x000000);
  myDelay(300); 		                                                  /* 延时                         */
	Board_LED_Set(0,true); 
	bufferInit();
  DEBUGOUT("Start to write sector 0...\n\r");
  flash_write_sector(0x000000, WrBuf, BUFFER_SIZE);                          /* 以0x0为起始地址，将WrBuf  */
  myDelay(30);
    
	DEBUGOUT("Start to read sector 0...\n\r");                                                                    /* 数组里的20个数据写入芯片     */
	flash_read_data(0x000000,  RdBuf,BUFFER_SIZE);                            /* 以0x0为起始地址，读20个   */
     
	if(bufferVerify()==0)
	{
    Board_LED_Set(2,true);                                                         /*  若SPI读写正确，D4常亮 */
		DEBUGOUT("Verify ok...");
	}
	while(1) {
    };
	

}
