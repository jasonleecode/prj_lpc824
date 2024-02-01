I2C Bus Looping Example
==========================================

Example Description
-------------------
This example shows how to configure I2C as a bus master and slave at the same time looping data transfer.
it configures I2C0 as bus master and I2C1 as bus slave. Master sends data to slave firstly and then reads the data back.
The data sent and read back should be the same. After successful verification, a green led on the board will be flashed, otherwise it flashes the red led.

Special connection requirements
-------------------------------

Board [NXP_LPCXPRESSO_824]:
This example requires an LPCXpresso base-board to run.
P0_10 and P0_11 are the I2C0 bus pins, P0_17 and P0_13 are the I2C1 bus pins.
Connect P0_10 to p0_17, P0_11 to P0_13.

Build procedures:
Visit the <a href="http://www.lpcware.com/content/project/lpcopen-platform-nxp-lpc-microcontrollers/lpcopen-v200-quickstart-guides">LPCOpen quickstart guides</a>
to get started building LPCOpen projects.
