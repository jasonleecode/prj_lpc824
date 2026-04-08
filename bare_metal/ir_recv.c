/*
 * IR receiver driver for LPC824 bare-metal
 * Protocol: NEC (standard 32-bit frame)
 *
 * NEC timing (demodulated receiver output, active LOW = burst):
 *   Leader mark  : 9000 µs LOW
 *   Leader space : 4500 µs HIGH  -> data follows
 *   Repeat space : 2250 µs HIGH  -> repeat code
 *   Bit mark     :  560 µs LOW   (all bits)
 *   Bit "0" space:  560 µs HIGH
 *   Bit "1" space: 1690 µs HIGH
 *   Stop mark    :  560 µs LOW
 *
 * The PININT0 fires on both edges; in the ISR we measure the elapsed
 * time since the previous edge using MRT channel 0 (one-shot, counts
 * down at 30 MHz = 30 counts/µs).
 */

#include "lpc_chip/chip.h"
#include "ir_recv.h"

/* -----------------------------------------------------------------------
 * Timing constants (µs)
 * ----------------------------------------------------------------------- */
#define NEC_LEADER_MARK_MIN   8000u
#define NEC_LEADER_MARK_MAX  10000u
#define NEC_LEADER_SPC_MIN    4000u
#define NEC_LEADER_SPC_MAX    5000u
#define NEC_REPEAT_SPC_MIN    1800u
#define NEC_REPEAT_SPC_MAX    2800u
#define NEC_BIT_MARK_MIN       350u
#define NEC_BIT_MARK_MAX       800u
#define NEC_ZERO_SPC_MIN       350u
#define NEC_ZERO_SPC_MAX       800u
#define NEC_ONE_SPC_MIN       1400u
#define NEC_ONE_SPC_MAX       1900u
#define NEC_FRAME_TIMEOUT    15000u   /* reset state if silent > 15 ms */

/* -----------------------------------------------------------------------
 * MRT helpers
 * The header wrongly marks TIMER as __O (write-only); it is read-only.
 * Use a volatile cast to read it.
 * ----------------------------------------------------------------------- */
#define MRT_TIMER_READ(ch)  (*(volatile const uint32_t *)(&LPC_MRT->CHANNEL[(ch)].TIMER))
#define MRT_MAX_COUNT       0x7FFFFFFFu
#define MRT_COUNTS_PER_US   (SystemCoreClock / 1000000u)  /* 30 at 30 MHz */

/* Reload the MRT one-shot timer with maximum value */
static void mrt_reload(void)
{
    /* MRT_INTVAL_LOAD (bit31) forces immediate reload */
    LPC_MRT->CHANNEL[0].INTVAL = MRT_MAX_COUNT | MRT_INTVAL_LOAD;
}

/* Return elapsed microseconds since last mrt_reload() call */
static uint32_t mrt_elapsed_us(void)
{
    uint32_t remaining = MRT_TIMER_READ(0);
    uint32_t counts = MRT_MAX_COUNT - remaining;
    return counts / MRT_COUNTS_PER_US;
}

/* -----------------------------------------------------------------------
 * Decoder state machine
 * ----------------------------------------------------------------------- */
typedef enum {
    IR_IDLE,          /* waiting for first falling edge */
    IR_LEADER_LOW,    /* measuring 9 ms leader mark */
    IR_LEADER_HIGH,   /* measuring 4.5 ms or 2.25 ms leader space */
    IR_DATA_LOW,      /* measuring ~560 µs bit mark */
    IR_DATA_HIGH,     /* measuring bit space (determines 0 or 1) */
} ir_state_t;

static volatile ir_state_t  s_state     = IR_IDLE;
static volatile uint32_t    s_raw_code  = 0;
static volatile uint8_t     s_bit_cnt   = 0;
static volatile bool        s_code_rdy  = false;
static volatile uint32_t    s_last_code = 0;

/* -----------------------------------------------------------------------
 * PININT0 ISR - fires on every edge of the IR input pin
 * ----------------------------------------------------------------------- */
void PININT0_IRQHandler(void)
{
    uint32_t elapsed;
    bool is_fall, is_rise;

    /* Determine which edge(s) fired */
    is_rise = (Chip_PININT_GetRiseStates(LPC_PININT) & PININTCH(IR_PININT_CH)) != 0;
    is_fall = (Chip_PININT_GetFallStates(LPC_PININT) & PININTCH(IR_PININT_CH)) != 0;

    /* Clear interrupt flags */
    Chip_PININT_ClearRiseStates(LPC_PININT, PININTCH(IR_PININT_CH));
    Chip_PININT_ClearFallStates(LPC_PININT, PININTCH(IR_PININT_CH));
    Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(IR_PININT_CH));

    elapsed = mrt_elapsed_us();
    mrt_reload();  /* restart timer for next interval */

    /* Timeout guard: reset state machine if signal was silent too long */
    if (elapsed > NEC_FRAME_TIMEOUT && s_state != IR_IDLE) {
        s_state = IR_IDLE;
    }

    switch (s_state) {

    /* --- Waiting: only care about a falling edge (burst start) --- */
    case IR_IDLE:
        if (is_fall) {
            s_state = IR_LEADER_LOW;
        }
        break;

    /* --- Measuring 9 ms leader mark (LOW): look for rising edge --- */
    case IR_LEADER_LOW:
        if (is_rise) {
            if (elapsed >= NEC_LEADER_MARK_MIN && elapsed <= NEC_LEADER_MARK_MAX) {
                s_state = IR_LEADER_HIGH;
            } else {
                s_state = IR_IDLE;
            }
        }
        break;

    /* --- Measuring leader space (HIGH): determines data vs repeat --- */
    case IR_LEADER_HIGH:
        if (is_fall) {
            if (elapsed >= NEC_LEADER_SPC_MIN && elapsed <= NEC_LEADER_SPC_MAX) {
                /* Normal frame: prepare to receive 32 data bits */
                s_raw_code = 0;
                s_bit_cnt  = 0;
                s_state    = IR_DATA_LOW;
            } else if (elapsed >= NEC_REPEAT_SPC_MIN && elapsed <= NEC_REPEAT_SPC_MAX) {
                /* Repeat code: re-use last code */
                s_last_code = s_last_code;   /* already stored */
                s_code_rdy  = true;
                s_state     = IR_IDLE;
            } else {
                s_state = IR_IDLE;
            }
        }
        break;

    /* --- Measuring ~560 µs bit mark (LOW): look for rising edge --- */
    case IR_DATA_LOW:
        if (is_rise) {
            if (elapsed >= NEC_BIT_MARK_MIN && elapsed <= NEC_BIT_MARK_MAX) {
                s_state = IR_DATA_HIGH;
            } else {
                s_state = IR_IDLE;
            }
        }
        break;

    /* --- Measuring bit space (HIGH): determines bit value --- */
    case IR_DATA_HIGH:
        if (is_fall) {
            uint32_t bit;
            if (elapsed >= NEC_ZERO_SPC_MIN && elapsed <= NEC_ZERO_SPC_MAX) {
                bit = 0;
            } else if (elapsed >= NEC_ONE_SPC_MIN && elapsed <= NEC_ONE_SPC_MAX) {
                bit = 1;
            } else {
                /* Invalid timing, abort */
                s_state = IR_IDLE;
                break;
            }

            /* NEC sends LSB first within each byte */
            s_raw_code |= (bit << s_bit_cnt);
            s_bit_cnt++;

            if (s_bit_cnt == 32) {
                /* All 32 bits received; the stop bit (rising edge after
                 * last mark) is not critical to decode, so mark as ready. */
                s_last_code = s_raw_code;
                s_code_rdy  = true;
                s_state     = IR_IDLE;
            } else {
                s_state = IR_DATA_LOW;
            }
        }
        break;

    default:
        s_state = IR_IDLE;
        break;
    }
}

/* -----------------------------------------------------------------------
 * Public API
 * ----------------------------------------------------------------------- */

void IR_Init(void)
{
    /* 1. Enable clocks */
    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_GPIO);
    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_MRT);

    /* 2. Configure IR pin as input with internal pull-up */
    Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, 0, IR_RECV_PIN);
    Chip_IOCON_PinSetMode(LPC_IOCON, IOCON_PIO1, PIN_MODE_PULLUP);

    /* 3. Map GPIO pin to PININT channel */
    Chip_SYSCTL_SetPinInterrupt(IR_PININT_CH, IR_RECV_PIN);

    /* 4. Configure PININT channel 0 for both edges */
    Chip_PININT_SetPinModeEdge(LPC_PININT, PININTCH(IR_PININT_CH));
    Chip_PININT_EnableIntHigh(LPC_PININT, PININTCH(IR_PININT_CH));   /* rising */
    Chip_PININT_EnableIntLow(LPC_PININT, PININTCH(IR_PININT_CH));    /* falling */

    /* 5. Enable PININT0 interrupt in NVIC */
    NVIC_SetPriority(PIN_INT0_IRQn, 1);
    NVIC_EnableIRQ(PIN_INT0_IRQn);

    /* 6. Start MRT channel 0 in one-shot mode (no interrupt, just timing) */
    Chip_MRT_Init();
    Chip_MRT_SetMode(LPC_MRT_CH0, MRT_MODE_ONESHOT);
    mrt_reload();
}

bool IR_CodeReady(void)
{
    return s_code_rdy;
}

uint32_t IR_GetCode(void)
{
    s_code_rdy = false;
    return s_last_code;
}

void IR_Decode(uint32_t raw, ir_nec_frame_t *frame)
{
    frame->addr     = (uint8_t)(raw >>  0);
    frame->addr_inv = (uint8_t)(raw >>  8);
    frame->cmd      = (uint8_t)(raw >> 16);
    frame->cmd_inv  = (uint8_t)(raw >> 24);
}
