# RT-Thread Nano for LPC824Lite

基于 RT-Thread Nano 实时操作系统的 LPC824Lite 开发板项目

## 项目信息

- **操作系统**: RT-Thread Nano 5.0.1
- **目标芯片**: NXP LPC824 (Cortex-M0+, 30MHz)
- **Flash**: 32KB | **RAM**: 8KB
- **工具链**: arm-none-eabi-gcc v15.1.0
- **构建系统**: SCons + Python

## 硬件资源

### 片上外设
- **UART**: UART0/1/2（波特率可配置，默认 115200 bps）
- **I2C**: I2C0（400 KHz）
- **SPI**: SPI0
- **SCT**: 状态可配置定时器（支持 PWM 输出）
- **GPIO**: 32 个通用 I/O 引脚

### 板载设备
- **EEPROM**: AT24C02 (2KB, I2C 地址 0x50)
- **Flash**: W25Q32 (4MB, SPI)
- **OLED**: SSD1306 (128×64, I2C 地址 0x3C)
- **按键**: 4 个方向按键（UP/DOWN/LEFT/RIGHT）
- **LED**: 8 个 LED (P0_7, P0_13, P0_16, P0_17, P0_19, P0_27, P0_28, P0_18)

## 项目结构

```
os_rtt/
├── bsp/                    # 板级支持包
│   ├── drivers/            # 驱动代码
│   │   ├── board.c         # 板级初始化
│   │   ├── board.h         # 板级定义 + LED 宏
│   │   ├── usart.c         # UART 驱动 + 串口命令处理
│   │   ├── drv_i2c.c       # I2C 总线驱动
│   │   ├── drv_spi.c       # SPI 总线驱动
│   │   ├── drv_pwm.c       # PWM 驱动 (6 通道)
│   │   ├── drv_at24c02.c   # EEPROM 驱动
│   │   ├── drv_w24q32.c    # Flash 驱动
│   │   ├── drv_oled.c      # OLED 驱动
│   │   └── drv_key.c       # 按键驱动
│   ├── applications/       # 应用代码
│   │   └── application.c   # 主应用
│   ├── Libraries/          # LPC824 库文件
│   ├── rtconfig.h          # 配置文件
│   ├── rtconfig.py         # 编译配置
│   ├── show_size.py        # 编译后显示内存占用
│   ├── SConscript          # SCons 构建脚本
│   └── SConstruct          # SCons 主脚本
├── src/                    # RT-Thread 内核源码
├── include/                # RT-Thread 头文件
├── components/             # 组件库
├── tools/                  # 工具脚本
└── README.md              # 本文件
```

## 编译和运行

### 前置条件
```bash
# macOS
brew install arm-linux-gcc
pip install kconfiglib

# Ubuntu/Debian
sudo apt-get install gcc-arm-none-eabi
pip install kconfiglib
```

### 编译
```bash
cd os_rtt/bsp
scons
```

编译完成后会自动显示内存占用情况：
```
╔════════════════════════════════════════╗
║ Flash:   9272/ 32768 bytes ( 28.3%) ║
║ RAM:      648/  8192 bytes (  7.9%) ║
╚════════════════════════════════════════╝
```

### 烧写
LPC824Lite 支持 IAP 方式烧写，编译生成的 `rtthread.bin` 可直接拖拽到开发板的虚拟磁盘进行烧写。

## 关键功能

### 1. UART 命令处理
支持通过串口发送命令进行电源管理：
- `help` - 显示帮助信息
- `sleep` - 进入睡眠模式
- `deep_sleep` - 进入深睡眠模式
- `power_down` - 进入掉电模式
- `deep_powerdown` - 进入深掉电模式

### 2. 电源管理
四种低功耗模式：
- **Sleep**: 内核停止，外设和内存活跃
- **Deep Sleep**: 外设无内部时钟，Flash 待机
- **Power Down**: Flash 也掉电
- **Deep Power Down**: 极低功耗，需要重启

API:
```c
void board_enter_sleep(void);
void board_enter_deep_sleep(void);
void board_enter_power_down(void);
void board_enter_deep_powerdown(void);
```

### 3. PWM 驱动
6 个独立 PWM 通道，频率范围 1-10000 Hz：
```c
rt_hw_pwm_init("pwm", channel, frequency, duty_cycle);
pwm_set_frequency(dev, frequency);
pwm_set_duty_cycle(dev, cycle);
pwm_enable(dev);
pwm_disable(dev);
```

### 4. LED 控制
8 个 LED 已在 board.h 中定义宏，使用示例：
```c
#define LED0_IO  (LPC_GPIO_PORT_PIN(0, 7))
#define LED1_IO  (LPC_GPIO_PORT_PIN(0, 13))
// ... 以此类推
```

## 配置说明

### UART 波特率配置
在 `rtconfig.h` 中修改：
```c
#define RT_SERIAL_BAUDRATE 115200  // 默认 115200 bps
```

### GPIO 和驱动配置
通过 menuconfig 进行图形化配置：
```bash
cd os_rtt/bsp
python3 ../../tools/kconfig/menuconfig.py
```

或直接编辑 `rtconfig.h` 中的配置宏。

## RT-Thread 版本信息

| 项目 | 值 |
|------|-----|
| RT-Thread 版本 | 5.0.1 |
| 版本代码 | 0x50001 (hex) |
| 主版本号 | 5 |
| 次版本号 | 0 |
| 修订号 | 1 |

版本定义位置：
- **源头**: `include/rtdef.h` (RT_VERSION_MAJOR, RT_VERSION_MINOR, RT_VERSION_PATCH)
- **配置**: `bsp/rtconfig.h` (RT_VER_NUM)
- **Kconfig**: `src/Kconfig` (RT_VER_NUM)

## 调试信息

### 编译输出
- `rtthread-lpc842.elf` - 带符号的可执行文件 (~25 KB)
- `rtthread.bin` - 烧写用的二进制文件 (~9.1 KB)
- `rtthread-lpc824.map` - 链接映射文件

### 内存占用
编译后自动显示：
- **Flash**: 代码 + 初始化数据
- **RAM**: 初始化数据 + 未初始化数据

## 常见问题

### Q: 如何修改波特率？
A: 编辑 `bsp/rtconfig.h`，修改 `RT_SERIAL_BAUDRATE` 宏。

### Q: 如何添加新的 LED 或改变引脚？
A: 编辑 `bsp/drivers/board.h`，更新 LEDx_PIN 的值。

### Q: 如何启用新的驱动或功能？
A: 使用 menuconfig 进行配置，或直接编辑 `rtconfig.h`。

### Q: 编译错误怎么排查？
A: 
1. 确保工具链已安装：`arm-none-eabi-gcc --version`
2. 清理编译输出：`scons -c`
3. 重新编译：`scons`

## 开发记录

- **2025-01-16**: 修复 UART 驱动中的未定义变量 bug，添加波特率配置宏，实现编译后自动显示内存占用百分比

## 相关资源

- [RT-Thread 官方网站](https://www.rt-thread.org/)
- [LPC824 数据手册](ucDragon_Official_Docs/芯片手册/)
- [优龙官方文档](ucDragon_Official_Docs/)

## 许可证

本项目遵循 RT-Thread 的许可证要求。

