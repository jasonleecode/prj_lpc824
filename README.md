# prj_lpc824
LPC824Lite这个开发板是我2016年（也可能是2015年，时间久了有点记不清）参加rt-thread在北航(BUAA)举行的沙龙活动上抽奖得到的，但是一直没有时间把玩。这个板子是优龙科技使用NXP的LPC824芯片开发的，基于Cortex-M0核心，主频30MHz，32KB片内Flash，8KB RAM，板载资源丰富，带一颗2KB I2C EEPROM(AT24C02)和一颗4MB SPI Flash(W24Q32)。现在将它的裸机程序和基于rt-thread的程序整理在这里。构建环境为ubuntu20.04。

LPC824Lite development board was won as a prize at the RT-Thread salon event held at Beihang University(BUAA) in 2016 (or possibly 2015, it's a bit hazy due to the passage of time). However, I haven't had the time to play with it until now. This board was developed by uCdragon Technology using NXP's LPC824 chip, based on the Cortex-M0 core with a main frequency of 30MHz, 32KB of on-chip Flash, and 8KB of RAM. It comes with abundant onboard resources, including a 2KB I2C EEPROM (AT24C02) and a 4MB SPI Flash (W24Q32). I've now organized its bare-metal program and the one based on RT-Thread here.Build environment is Ubuntu 20.04.

ucDragon_Official_Docs/lpc824lite.jpg
# 
<++>

# install toolchain
arm-linux-gcc
<++>


# build
`make
` <++>

<++>

# download and run


<++>
