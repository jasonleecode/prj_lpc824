UART调试字符串例子
========================================

示例描述
-------------------
这个示例演示如何通过调试UART口输出字符串

请把板子的micro USB接口连接到电脑上并打开一个终端仿真程序。PC上设置为115200-8-N-1.
复位板子后示例会先打印一个简单的消息到终端上，然后每秒钟输出当前的时钟tick值。

其它注意事项
-------------------------------
[NXP_LPCXPRESSO_812]开发板:

Board [NXP_812_MAX]开发板:

Board [NXP_LPCXPRESSO_824]开发板:

===================================================================================================

UART debug string example
========================================

Example Description
-------------------
The UART example shows how to use the debug UART to output strings.

To use the example, connect a serial cable to the board's micro USB port and
start a terminal program to monitor the port.  The terminal program on the host
PC should be setup for 115200-8-N-1.
Once the example is started, a brief message is sent to the terminal. Later the 
current value of tick is updated to terminal every second.

Special connection requirements
-------------------------------
Board [NXP_LPCXPRESSO_812]:

Board [NXP_812_MAX]:

Board [NXP_LPCXPRESSO_824]:
