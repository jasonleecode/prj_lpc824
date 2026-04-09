#include "lpc_chip/board.h"
#include "ir_recv.h"

/****************/
/* IR Receiver Demo */
/* 波特率：115200， LPC_USART0 */
/****************/


/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
#define TICKRATE_HZ 100

#define LED 0

static volatile uint32_t tick_ct = 0;

/*****************************************************************************
 * Private functions
 ****************************************************************************/
static void ansi_clr_screen(void) {
    Board_UARTPutSTR("\x1B[2J");    /* clear screen */
    Board_UARTPutSTR("\x1B[0m");    /* color mode reset */
    Board_UARTPutSTR("\x1B[1;1H"); /* set position to 1,1 */
}

static void Board_LED_clear(void) {
    Board_LED_Set(LED, false);
}

/* Print an unsigned 32-bit value in hex, zero-padded to 8 digits */
static void print_hex32(uint32_t val)
{
    static const char hex[] = "0123456789ABCDEF";
    char buf[9];
    int i;
    buf[8] = '\0';
    for (i = 7; i >= 0; i--) {
        buf[i] = hex[val & 0xF];
        val >>= 4;
    }
    Board_UARTPutSTR(buf);
}

/*****************************************************************************
 * Interrupt handlers
 ****************************************************************************/
void SysTick_Handler(void) {
    tick_ct++;
    if ((tick_ct % 100) == 0) Board_LED_Toggle(LED);
}

/*****************************************************************************
 * main
 ****************************************************************************/
int main(void)
{
    SystemCoreClockUpdate();
    Board_Init();
    Board_LED_clear();

    ansi_clr_screen();
    Board_UARTPutSTR("LPC824 IR Receiver Demo\r\n");
    Board_UARTPutSTR("build: " __DATE__ " " __TIME__ "\r\n");
    Board_UARTPutSTR("Connect 38kHz IR receiver OUT to P0.20\r\n\r\n");

    /* Enable SysTick at 100 Hz */
    SysTick_Config(SystemCoreClock / TICKRATE_HZ);

    /* Initialize IR receiver (P0.20, PININT0, MRT0) */
    IR_Init();

    while (1) {
        __WFI();   /* sleep until any interrupt */

        if (IR_CodeReady()) {
            uint32_t raw = IR_GetCode();
            ir_nec_frame_t frame;
            IR_Decode(raw, &frame);

            Board_UARTPutSTR("IR code: 0x");
            print_hex32(raw);

            Board_UARTPutSTR("  addr=0x");
            Board_UARTPutChar("0123456789ABCDEF"[frame.addr >> 4]);
            Board_UARTPutChar("0123456789ABCDEF"[frame.addr & 0xF]);

            Board_UARTPutSTR("  cmd=0x");
            Board_UARTPutChar("0123456789ABCDEF"[frame.cmd >> 4]);
            Board_UARTPutChar("0123456789ABCDEF"[frame.cmd & 0xF]);

            /* Simple NEC integrity check: addr ^ addr_inv should be 0xFF */
            if ((uint8_t)(frame.addr ^ frame.addr_inv) == 0xFF &&
                (uint8_t)(frame.cmd  ^ frame.cmd_inv)  == 0xFF) {
                Board_UARTPutSTR("  [OK]");
            } else {
                Board_UARTPutSTR("  [EXT/ERR]");
            }

            Board_UARTPutSTR("\r\n");
        }
    }
}
