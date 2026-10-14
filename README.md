# STM32 USB HID 游戏手柄

基于 **STM32F103C8Tx** 开发的 USB HID 游戏手柄固件。设备通过 USB 连接电脑后，可作为标准游戏控制器使用，无需安装专用驱动。

项目采用分层模块化设计，将硬件驱动、通用算法与应用逻辑分离，便于调试、维护和功能扩展。

## 功能特性

- USB HID 游戏手柄设备
- 15 个独立按键输入
- 双轴模拟摇杆采集
- ADC + DMA 连续采样
- SSD1306 OLED 状态显示
- LED 状态指示
- 有源蜂鸣器提示
- 按键消抖与状态机处理
- 摇杆滤波、死区处理和数值映射
- 模块化 BSP / Common / App 工程结构

## 硬件平台

| 项目 | 配置 |
| --- | --- |
| MCU | STM32F103C8Tx |
| 内核 | ARM Cortex-M3 |
| 主频 | 72 MHz |
| USB | USB Full Speed Device（PA11 / PA12） |
| 调试接口 | SWD |
| 开发工具 | STM32CubeMX 6.11.1、Keil MDK-ARM V5.32 |

## 引脚说明

### 模拟摇杆

| 功能 | STM32 引脚 | 外设通道 |
| --- | --- | --- |
| 摇杆 X 轴 | PA0 | ADC1_IN0 |
| 摇杆 Y 轴 | PA1 | ADC1_IN1 |

ADC1 配置为扫描模式，使用 DMA1 Channel 1 循环搬运两个通道的采样结果。

### 按键

按键均配置为上拉输入，按下时为低电平。

| 编号 | STM32 引脚 | 编号 | STM32 引脚 | 编号 | STM32 引脚 |
| --- | --- | --- | --- | --- | --- |
| Button 1 | PA2 | Button 6 | PA7 | Button 11 | PB13 |
| Button 2 | PA3 | Button 7 | PB0 | Button 12 | PB14 |
| Button 3 | PA4 | Button 8 | PB1 | Button 13 | PB15 |
| Button 4 | PA5 | Button 9 | PB10 | Button 14 | PA8 |
| Button 5 | PA6 | Button 10 | PB11 | Button 15 | PA15 |

### 其他外设

| 功能 | STM32 引脚 | 说明 |
| --- | --- | --- |
| OLED SCL | PB6 | I²C1 时钟线 |
| OLED SDA | PB7 | I²C1 数据线 |
| LED | PB8 | 低电平点亮 |
| 有源蜂鸣器 | PB9 | 低电平有效 |
| USB D- | PA11 | USB Full Speed |
| USB D+ | PA12 | USB Full Speed |
| SWDIO | PA13 | 调试接口 |
| SWCLK | PA14 | 调试接口 |

## 软件架构

工程按照“硬件驱动层 → 通用算法层 → 应用层”的方式组织：

```text
joystick/
├── App/                         # 应用层
│   ├── app_gamepad.c/.h         # 手柄主流程与 HID 数据上报
│   └── app_types.h              # 应用层公共类型
├── Bsp/                         # 板级支持包
│   ├── bsp_adc.c/.h             # ADC + DMA 摇杆采集
│   ├── bsp_beep.c/.h            # 蜂鸣器驱动
│   ├── bsp_button.c/.h          # 15 路按键扫描
│   ├── bsp_led.c/.h             # LED 驱动
│   └── bsp_oled.c/.h            # OLED 驱动
├── Common/                      # 通用算法与工具
│   ├── button_fsm.c/.h          # 按键消抖状态机
│   ├── filter.c/.h              # 滤波算法
│   ├── joystick_process.c/.h    # 摇杆死区与映射
│   └── utils.c/.h               # 通用工具函数
├── Core/                        # STM32CubeMX 生成的核心代码
├── Drivers/                     # STM32 HAL/CMSIS 驱动
├── Middlewares/                 # USB Device 中间件
├── USB_DEVICE/                  # USB HID 设备配置
├── joystick.ioc                 # STM32CubeMX 工程配置
├── joystick.uvprojx             # Keil MDK 工程文件
└── STM32游戏手柄工程架构说明.md   # 详细工程架构文档
```

详细的模块职责、接口约定和初始化顺序请参阅 [STM32游戏手柄工程架构说明.md](./STM32游戏手柄工程架构说明.md)。

## 工作流程

设备上电后的主要处理流程如下：

1. 初始化 HAL、系统时钟和 GPIO。
2. 初始化 DMA、ADC、I²C 和 USB Device。
3. 初始化 LED、蜂鸣器、OLED、按键和摇杆处理模块。
4. 在主循环中扫描按键并读取摇杆 ADC 数据。
5. 对摇杆数据进行滤波、死区处理和数值映射。
6. 更新 OLED 显示及 LED、蜂鸣器状态。
7. 通过 USB HID 向电脑周期性上报手柄状态。

## 编译与烧录

### 环境要求

- Keil MDK-ARM 5.x
- STM32F1 Device Family Pack
- ST-Link 或兼容的 SWD 下载器
- STM32CubeMX（仅在修改外设配置或重新生成代码时需要）

### 操作步骤

1. 克隆仓库：

   ```bash
   git clone https://github.com/2918223672/joystick.git
   cd joystick
   ```

2. 使用 Keil MDK 打开 `joystick.uvprojx`。
3. 选择工程目标 `joystick`，执行 Build 编译。
4. 通过 ST-Link 连接开发板的 SWDIO、SWCLK、GND 和电源。
5. 在 Keil 中配置下载器并执行 Download。
6. 复位开发板，然后使用 USB 数据线连接电脑。

## 使用方法

1. 将固件烧录到 STM32F103C8Tx 开发板。
2. 使用支持数据传输的 USB 线连接开发板和电脑。
3. 等待系统识别 USB HID 游戏控制器。
4. 在 Windows 中按 `Win + R`，输入 `joy.cpl`，打开“游戏控制器”面板。
5. 选择识别到的设备，进入“属性”页面测试摇杆与按键。

如果系统无法识别设备，请优先检查：

- PA11、PA12 的 USB 接线是否正确；
- USB 数据线是否支持数据传输；
- 系统时钟和 USB 时钟配置是否正确；
- 固件是否成功烧录并正常运行；
- USB HID 描述符与报告长度是否一致。

## 二次开发建议

- 修改按键数量或映射时，同步调整 `bsp_button` 与 USB HID 报告描述符。
- 修改摇杆量程时，在 `joystick_process` 中调整中心值、死区和输出映射范围。
- 重新使用 STM32CubeMX 生成代码前，建议先提交或备份自定义模块，避免覆盖用户代码。
- 新增硬件驱动放入 `Bsp/`，通用算法放入 `Common/`，业务逻辑放入 `App/`。

## 相关文档

- [工程架构说明](./STM32游戏手柄工程架构说明.md)
- [STM32CubeMX 工程配置](./joystick.ioc)

## 项目状态

项目目前已具备 USB HID 手柄的基础工程框架，可继续扩展按键映射、摇杆校准、连发、组合键、振动反馈和配置工具等功能。
