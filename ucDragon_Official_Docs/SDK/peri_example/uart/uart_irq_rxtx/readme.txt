基于中断的USART发送与接收示例
========================================

示例描述
-------------------
这个示例演示如何在同一个USART上同时使用中断模式进行发送与接收
	它使用"环回"模式，因此USART自发自收。
	发送部分: 读取Flash内容并且组织成数据包依次发送。全部发送完后回卷到0地址继续发送。
	接收部分: 读取收到的数据字节并且重新组织成数据包。
	主循环: 当SW2键被按下时，调用发送部分。
	USART ISR: 处理RXRDY和TXRDY/TXIDLE事件，以推动数据包的发送全过程。
		注:　TXRDY和TXIDLE可以任选一个使用，但是不要同时使用。理论上TXRDY会使发送更快一点，因为它使当前字节发完之前就准备下个字节。
这个例子也演示了如何恰当地使能和禁用TXRDY/TXIDLE中断，如何在ISR与后台程序(主循环)中同步
这个例子使用调试UART来打印内部运行状态信息。
这个例子使用SysTick来测量发送一个数据包所消耗的时间。

要观察示例的输出，请把板子的micro USB端口连接到PC上(在LPC824-MAX板上有板载仿真器，它自带了USB转串口的功能), 打开一个终端仿真程序来观察。
PC上的设置是115200-N-8-1.
在示例启动后，会等候用户按下SW2键。当SW2键被按下后，就开始发送数据包，同时打印跟踪信息，包括数据包的发送耗时，接收是否成功。

其它注意事项
-------------------------------
[NXP_LPCXPRESSO_812]开发板:

Board [NXP_812_MAX]开发板:

Board [NXP_LPCXPRESSO_824]开发板:

===================================================================================================

UART Interrupt for both RX and TX example
========================================

Example Description
-------------------
	It uses "Loopback" mode so USART receives whatever it sent.
	TX part: Reads flash content and sends out packet by packet. When all data are sent, it rolls back to 0 and do again.
	RX part: Receives data byte by byte, and rebuild the packet.
	Mainloop: When key SW2 is pressed down, it calls TX part.
	USART ISR: It processes RXRDY and TXRDY/TXIDLE events and thus continues packet sending and receiving.
		Note: Either TXRDY or TXIDLE can be used, but not both at a given time. In theory, TXRDY should be slightly faster.
This example also shows how to enable/disable the TXRDY/TXIDLE IRQ properly.
This example also shows how to synchornize between ISR and background (main loop).
This example uses debug UART to print trace logs
This example uses Systick to measure how much time is taken to send one packet.

To use the example, connect a serial cable to the board's debug UART port and
start a terminal program to monitor the port.  The terminal program on the host
PC should be setup for 115200-8-N-1.
Once the example is started, it detects SW2 key's state in main loop, once SW2 is pressed, it sends data packets, and print
the trace logs, including elpased time for sending one packet, and if receiving is successful.

Special connection requirements
-------------------------------
Board [NXP_LPCXPRESSO_812]:

Board [NXP_812_MAX]:

Board [NXP_LPCXPRESSO_824]:
