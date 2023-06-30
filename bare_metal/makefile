# Minimal gcc makefile for LPC824

# default Linux USB device name for upload
TTY ?= /dev/ttyACM*

# use the arm cross compiler, not std gcc
TRGT = arm-none-eabi-
CC = $(TRGT)gcc
CXX = $(TRGT)g++
CP = $(TRGT)objcopy
DUMP = $(TRGT)objdump

# compiler and linker settings
CFLAGS = -mcpu=cortex-m0plus -mthumb -I./ -Os -ggdb
CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions
LDFLAGS = -Wl,--script=./include/lpc824.ld -nostartfiles

# permit including this makefile from a sibling directory
vpath %.c lpc_chip:  startup: Include
vpath %.h lpc_chip:  startup: Include

# default target
upload: firmware.bin
	./lpc21isp/lpc21isp -control -donotstart -bin $< $(TTY) 115200 0

dump: firmware.elf
	$(DUMP) -d $< > firmware.s

firmware.elf: main.o gcc_startup_lpc82x.o board_sysinit.o board.o board_api_stubs_8xx.o crp.o mtb.o sysinit.o acmp_8xx.o  adc_8xx.o  board.o  board_sysinit.o  chip_8xx.o  clock_8xx.o  crc_8xx.o  dma_8xx.o  eeprom.o  gpio_8xx.o  i2c_common_8xx.o  i2cm_8xx.o  i2cs_8xx.o  iap.o  iocon_8xx.o  irc_8xx.o  pinint_8xx.o  pmu_8xx.o  ring_buffer.o  rtc_ut.o  sct_8xx.o  sct_pwm_8xx.o  spi_8xx.o  spim_8xx.o  spis_8xx.o  stopwatch_8xx.o  swm_8xx.o  syscon_8xx.o  sysinit_8xx.o  uart_8xx.o  wkt_8xx.o  wwdt_8xx.o 
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^

%.bin: %.elf
	$(CP) -O binary $< $@

clean:
	rm -f *.o *.elf *.bin *.s

# these target names don't represent real files
.PHONY: upload dump clean
