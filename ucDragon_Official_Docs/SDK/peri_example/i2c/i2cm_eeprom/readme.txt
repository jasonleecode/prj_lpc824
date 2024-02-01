I2C Bus Master (interrupt) Example
==========================================

Example Description
-------------------
This example shows how to configure I2C as a bus master in interrupt mode using
the I2CM driver. This example uses 7-bit addressing to write and read EEPROM AT24C02 on the FRDM mini shield board. After I2C is setup, the I2CM master transmit functions are called through the i2cm_8xx driver routines.

Notes:
1)	This example does not use the RS-232 port.

2)	The LPCXpresso base-board does not require power to run this example.

3)	The I2C address: 0x50 for AT24C02

Special connection requirements
-------------------------------

Board [NXP_LPCXPRESSO_824]:
This example requires an LPCXpresso base-board and FRDM mini shield board to run.

Build procedures:
Visit the <a href="http://www.lpcware.com/content/project/lpcopen-platform-nxp-lpc-microcontrollers/lpcopen-v200-quickstart-guides">LPCOpen quickstart guides</a>
to get started building LPCOpen projects.
