/*
 * IR receiver driver for LPC824 bare-metal
 * Protocol: NEC (9ms leader + 32-bit data)
 * Input pin: P0.20 (demodulated 38kHz IR receiver, e.g. VS1838B)
 */

#ifndef IR_RECV_H
#define IR_RECV_H

#include <stdint.h>
#include <stdbool.h>

/* IR receiver GPIO pin configuration - change here if needed */
#define IR_RECV_PIN     20      /* P0.20 */
#define IR_PININT_CH    0       /* PININT channel 0 */

/* NEC frame fields extracted from a decoded 32-bit code */
typedef struct {
    uint8_t addr;       /* address byte */
    uint8_t addr_inv;   /* inverted address byte */
    uint8_t cmd;        /* command byte */
    uint8_t cmd_inv;    /* inverted command byte */
} ir_nec_frame_t;

/*
 * Initialize IR receiver:
 *   - Configure P0.20 as input with pull-up
 *   - Enable PININT0 on both edges
 *   - Initialize MRT channel 0 as microsecond timer
 */
void IR_Init(void);

/* Returns true if a new IR code has been received since last call */
bool IR_CodeReady(void);

/*
 * Return the last received 32-bit NEC code and clear the ready flag.
 * Bit layout (LSB first per byte):
 *   [31:24] ~cmd   [23:16] cmd   [15:8] ~addr   [7:0] addr
 */
uint32_t IR_GetCode(void);

/* Decode a raw 32-bit NEC code into address/command fields */
void IR_Decode(uint32_t raw, ir_nec_frame_t *frame);

#endif /* IR_RECV_H */
