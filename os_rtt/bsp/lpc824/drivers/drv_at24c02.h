#include <rtthread.h>
#include <rtdevice.h>

bool at24cx_isconnected(void);
static int32_t at24cx_read(uint32_t addr, uint8_t *buf);
static int32_t at24cx_write(uint32_t addr, uint8_t *buf);
bool at24cx_erasechip(void);