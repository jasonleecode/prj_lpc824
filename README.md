# prj_lpc824

LPC824Lite 这个开发板是我 2016 年（也可能是 2015 年，时间久了有点记不清）参加 rt-thread 在北航(BUAA)举行的沙龙活动上抽奖得到的，但一直没有时间把玩。这个板子是优龙科技使用 NXP 的 LPC824 芯片开发的，基于 Cortex-M0 核心，主频 30MHz，32KB 片内 Flash，8KB RAM，板载资源丰富，带一颗 2KB I2C EEPROM(AT24C02) 和一颗 4MB SPI Flash(W25Q32)。现在将它的裸机程序和基于 RT-Thread Nano 的程序整理在这里。

LPC824Lite development board was won as a prize at the RT-Thread salon event held at Beihang University (BUAA) in 2016 (or possibly 2015, it's a bit hazy). The board was developed by uCdragon Technology using NXP's LPC824 chip, based on the Cortex-M0 core with a main frequency of 30MHz, 32KB of on-chip Flash, and 8KB of RAM. It comes with abundant onboard resources, including a 2KB I2C EEPROM (AT24C02) and a 4MB SPI Flash (W25Q32). I've now organized both the bare-metal program and the RT-Thread Nano-based program here.

![image](ucDragon_Official_Docs/lpc824lite.jpg)

## 项目内容 | Contents

| 目录 | 说明 | Description |
|------|------|-------------|
| `bare_metal/` | 裸机程序 | Bare-metal firmware and demo |
| `os_rtt/` | RT-Thread Nano 实时操作系统 | RT-Thread Nano RTOS implementation |
| `ucDragon_Official_Docs/` | 官方文档和工具 | Official documentation and tools |

## 快速开始 | Quick Start

### 环境要求 | Requirements

```bash
# Install toolchain
brew install arm-linux-gcc  # macOS
# or
sudo apt-get install gcc-arm-none-eabi  # Linux

# Install build dependencies
pip install kconfiglib
```

### 编译和烧写 | Build and Flash

#### 裸机版本 | Bare-metal
```bash
cd bare_metal
make
# Output: rtthread.bin (可通过 IAP 烧写)
```

#### RT-Thread 版本 | RT-Thread Nano
```bash
cd os_rtt/bsp
scons
# Output: rtthread.bin (可通过 IAP 烧写)
```

编译完成后会自动显示内存占用情况。

### 烧写到开发板 | Download to Board

LPC824Lite 支持 IAP (In-Application Programming) 烧写方式，可直接将 bin 文件拖拽到开发板的虚拟磁盘。

## 项目特性 | Features

### 裸机版本 | Bare-metal
- ✅ 基础系统初始化和 SysTick
- ✅ UART 通信驱动
- ✅ LED 演示程序

### RT-Thread 版本 | RT-Thread Nano
- ✅ 完整的实时内核 (5.0.1)
- ✅ 多任务调度
- ✅ 中断管理和异常处理
- ✅ 内存管理和消息队列
- ✅ **UART 驱动** - 支持 3 个 UART (可配置波特率)
- ✅ **I2C 驱动** - I2C0 主模式
- ✅ **SPI 驱动** - SPI0 主模式
- ✅ **PWM 驱动** - 6 个独立通道 (1-10000 Hz)
- ✅ **EEPROM 驱动** - AT24C02 (I2C, 2KB)
- ✅ **Flash 驱动** - W25Q32 (SPI, 4MB)
- ✅ **OLED 驱动** - SSD1306 (I2C, 128×64)
- ✅ **按键驱动** - 4 方向按键 + 20ms 防抖
- ✅ **LED 定义** - 8 个 LED 的管脚宏
- ✅ **电源管理** - 4 种低功耗模式 + 串口命令控制
- ✅ **实时命令处理** - 通过串口发送命令控制系统

## 硬件规格 | Hardware Specs

| 项目 | 规格 |
|------|------|
| 芯片 | NXP LPC824 (Cortex-M0+) |
| 主频 | 30 MHz |
| Flash | 32 KB |
| SRAM | 8 KB |
| I2C | I2C0, I2C1 (x2) |
| SPI | SPI0, SPI1 (x2) |
| UART | UART0, UART1, UART2 (x3) |
| GPIO | 32 pins |
| 定时器 | SCT, MRT, WWDT |
| 其他 | ADC, CMP, 实时时钟 |

## 项目结构 | Project Structure

```
prj_lpc824/
├── bare_metal/
│   ├── main.c
│   ├── makefile
│   ├── include/
│   │   ├── LPC82x.h
│   │   ├── lpc824.ld
│   │   ├── lpc_types.h
│   │   └── core_*.h
│   └── build/
├── os_rtt/
│   ├── bsp/
│   │   ├── drivers/       # 驱动代码
│   │   ├── applications/  # 应用程序
│   │   ├── Libraries/     # NXP 库文件
│   │   ├── rtconfig.h
│   │   ├── rtconfig.py
│   │   ├── show_size.py   # 内存占用显示脚本
│   │   ├── SConscript
│   │   └── SConstruct
│   ├── src/               # RT-Thread 内核源码
│   ├── include/           # RT-Thread 头文件
│   ├── components/        # 可选组件
│   ├── tools/             # 工具脚本
│   ├── Kconfig
│   └── README.md          # 详见 os_rtt/README.md
├── ucDragon_Official_Docs/
│   ├── SDK/
│   ├── 使用手册/
│   ├── 原理图/
│   ├── 实用工具/
│   ├── 目标代码/
│   └── 芯片手册/
└── README.md              # 本文件
```

## 相关文档 | Documentation

- [os_rtt/README.md](os_rtt/README.md) - RT-Thread 实现的详细文档
- [官方文档](ucDragon_Official_Docs/) - 优龙官方资料和芯片手册

## 开发环境 | Build Environment

- **OS**: Ubuntu 20.04 / macOS Monterey+
- **工具链**: arm-none-eabi-gcc v15.1.0 (GCC ARM Embedded)
- **构建系统**: SCons (Python-based)
- **配置工具**: kconfiglib (Linux kernel menuconfig)

## 版本历史 | Version History

### 2025-01-16
- 🐛 修复 UART 驱动中的未定义变量 bug
- ✨ 添加波特率配置宏到 rtconfig.h
- ✨ 实现编译后自动显示 Flash/RAM 占用百分比
- 📝 完善 os_rtt README 文档

### 2025-01-15
- ✨ 实现 PWM 驱动 (6 通道)
- ✨ 完善 Kconfig 配置系统

### 2025-01-14
- ✨ 实现电源管理接口 (4 种低功耗模式)
- ✨ 添加串口命令实时处理

## 常见问题 | FAQ

### Q: 如何修改波特率？
A: 编辑 `os_rtt/bsp/rtconfig.h`，修改 `RT_SERIAL_BAUDRATE` 宏。

### Q: 如何使用电源管理功能？
A: 通过串口发送命令：`sleep`、`deep_sleep`、`power_down`、`deep_powerdown`

### Q: 如何控制 PWM？
A: 使用 drv_pwm.h 提供的 API，支持 6 个独立通道。

### Q: 如何配置硬件驱动？
A: 使用 menuconfig 或直接编辑 rtconfig.h 中的配置宏。

## 许可证 | License

本项目遵循 RT-Thread 的许可证要求。

## 相关链接 | References

- [RT-Thread 官网](https://www.rt-thread.org/)
- [NXP LPC824 数据手册](https://www.nxp.com/)
- [优龙科技官网](https://www.ucdragon.com/)

---

**最后更新**: 2025-01-16 | **Latest Update**: 2025-01-16
