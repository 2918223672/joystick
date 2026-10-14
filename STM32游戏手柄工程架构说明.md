# STM32 游戏手柄工程架构说明

## 1. 工程概述

本工程基于 `STM32F103C8T6`，使用 `HAL 库 + FreeRTOS/CMSIS-RTOS2` 实现一个多功能游戏手柄。

当前支持：

- 双摇杆和 LT/RT 模拟量采集
- 15 个 GPIO 按键采集
- 按键消抖
- 摇杆中位校准、滤波和死区处理
- 0.96 寸 SSD1306 OLED 显示
- USB HID 游戏手柄
- USART1 蓝牙按键数据发送
- USB/蓝牙自动模式判断
- PC13 状态灯每 500 ms 闪烁

## 2. 总体分层

工程采用五层结构：

```text
硬件
  ↓
HAL/CubeMX 外设层
  ↓
Bsp 硬件封装层
  ↓
Common 节点与话题通信层
  ↓
App 业务逻辑层
  ↓
FreeRTOS 任务调度
```

### 2.1 CubeMX/HAL 层

负责初始化 STM32 的底层外设：

- GPIO
- ADC1 + DMA
- USART1
- TIM1
- USB FS
- FreeRTOS

主要目录：

```text
Core/
USB_DEVICE/
Middlewares/
Drivers/
```

CubeMX 生成的代码负责硬件初始化，业务代码尽量放在 `App` 和 `Bsp` 中。

## 3. Bsp 硬件封装层

目录：

```text
Bsp/
```

### 3.1 `bsp_key.c/.h`

负责 GPIO 按键读取。

当前按键编号：

```text
0  右摇杆按键
1  左摇杆按键
2  RIGHT
3  DOWN
4  LEFT
5  UP
6  RB
7  LB
8  HOME
9  MENU
10 VIEW
11 Y
12 B
13 A
14 X
```

按键采用上拉输入，按下时读取到低电平，BSP 层统一转换为：

```text
1：按下
0：释放
```

### 3.2 `bsp_joystick.c/.h`

负责 ADC 摇杆和扳机采集。

硬件对应关系：

```text
PA0 -> 左摇杆 ADC
PA1 -> 左摇杆 ADC
PA2 -> 右摇杆 ADC
PA3 -> 右摇杆 ADC
PA4 -> LT
PA5 -> RT
```

处理流程：

```text
ADC1扫描
  ↓
DMA循环搬运
  ↓
上电采样校准
  ↓
一阶低通滤波
  ↓
摇杆死区处理
  ↓
LT/RT释放死区处理
  ↓
重新映射到0~4095
  ↓
发布JoystickData_t
```

摇杆中位通常统一为 `2048`。

LT/RT使用独立释放值和满按端点，避免实际 ADC 达不到 4095 时无法达到显示或 USB 的满量程。

### 3.3 `bsp_oled.c/.h`

负责 SSD1306 128x64 OLED。

当前使用 PB12/PB13 软件 I2C：

```text
PB12 -> OLED_SCL
PB13 -> OLED_SDA
```

OLED显示内容：

- 顶部 LT/RT 状态条
- 左右摇杆活动框
- 左右摇杆当前位置圆点

顶部扳机条逻辑：

```text
释放状态 -> 中心位置
LT按下   -> 向左移动
LT满按   -> 左侧填充消失
RT按下   -> 向右移动
RT满按   -> 右侧填满
```

OLED任务不会因为摇杆静止而停止刷新。扳机状态单独变化时，也会触发显示更新。

### 3.4 `bsp_uart.c/.h`

负责 USART1 通信：

```text
PA9  -> USART1_TX
PA10 -> USART1_RX
波特率：115200
```

当前蓝牙发送使用阻塞发送接口，按键事件频率较低，逻辑简单。

### 3.5 `bsp_led.c/.h`

负责 PC13 状态灯：

```text
PC13低电平 -> 点亮
PC13高电平 -> 熄灭
```

当前由 `App_RgbTask` 每 500 ms 翻转一次。

## 4. Common 节点与话题层

目录：

```text
Common/
```

这一层模仿 ROS 的节点和话题思想，但针对 STM32 做了静态内存实现，不使用动态内存。

### 4.1 节点

当前有效节点：

```text
NODE_INPUT       输入节点
NODE_OLED        OLED显示节点
NODE_USB         USB HID输出节点
NODE_BLUETOOTH   蓝牙输出节点
```

输入节点负责采集，输出节点负责发送或显示。

### 4.2 话题

当前主要话题：

```text
TOPIC_KEY_EVENT
TOPIC_JOYSTICK_DATA
```

按键消息：

```c
typedef struct
{
    uint8_t key_id;
    uint8_t pressed;
} KeyData_t;
```

摇杆消息：

```c
typedef struct
{
    uint16_t lx;
    uint16_t ly;
    uint16_t rx;
    uint16_t ry;
    uint16_t lt;
    uint16_t rt;
} JoystickData_t;
```

### 4.3 消息流向

按键消息：

```text
InputTask
  ↓ 发布 TOPIC_KEY_EVENT
  ├── USB节点订阅
  └── Bluetooth节点订阅
```

摇杆消息：

```text
InputTask
  ↓ 发布 TOPIC_JOYSTICK_DATA
  ├── OLED节点订阅
  └── USB节点订阅
```

每个订阅者拥有独立队列，因此 USB、OLED 和蓝牙互不直接依赖。

## 5. App 业务层

目录：

```text
App/
```

### 5.1 `app_input.c`

输入节点任务，主要职责：

1. 扫描全部 GPIO 按键；
2. 对按键进行连续采样消抖；
3. 只在按键状态变化时发布事件；
4. 读取经过 Bsp 处理的摇杆数据；
5. 发布摇杆话题。

当前输入扫描周期为 10 ms，按键连续稳定 2 次后确认，约为 20 ms 消抖。

### 5.2 `app_tasks.c`

包含其他业务任务：

#### `App_UsbTask`

负责：

- 订阅按键话题；
- 保存 16 位按键位图；
- 订阅摇杆话题；
- 生成 8 字节 HID 报告；
- 向电脑发送游戏手柄数据。

USB报告格式：

```text
Byte 0：左摇杆X，-127~127
Byte 1：左摇杆Y，-127~127
Byte 2：右摇杆X，-127~127
Byte 3：右摇杆Y，-127~127
Byte 4：LT，0~127
Byte 5：RT，0~127
Byte 6：按键位图低8位
Byte 7：按键位图高8位
```

#### `App_BluetoothTask`

负责：

- 订阅按键话题；
- 判断当前是否为蓝牙模式；
- 按自定义协议发送按键。

蓝牙数据包：

```text
0x55 + ASCII按键名称 + 0xAA
```

当前支持：

```text
UP、DOWN、LEFT、RIGHT
Y、B、X、A
```

LT/RT 按照设计不发送到蓝牙。

#### `App_OledTask`

负责：

- 订阅摇杆话题；
- 缓存最新的摇杆和扳机状态；
- 按固定刷新周期更新 OLED；
- 摇杆或 LT/RT 独立变化时都能触发刷新。

#### `App_RgbTask`

负责：

```text
每500 ms翻转一次PC13
```

## 6. USB/蓝牙自动模式

USB设备启动时，`MX_USB_DEVICE_Init()` 只负责启动 USB 协议栈：

```c
USBD_Init()
USBD_RegisterClass()
USBD_Start()
```

真正判断电脑是否识别成功，使用：

```c
hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED
```

模式判断：

```text
USB状态为CONFIGURED
    -> USB遥控器模式

USB未连接、未枚举或已断开
    -> 蓝牙模式
```

USB模式下：

```text
发送摇杆
发送LT/RT
发送LB/RB及其他按键
禁止蓝牙发送
```

蓝牙模式下：

```text
发送UP、DOWN、LEFT、RIGHT、Y、B、X、A
不发送LT/RT
```

这里判断的是“USB枚举是否成功”，而不是只判断 USB 线是否插入。

## 7. FreeRTOS任务

当前任务结构：

```text
InputTask
  -> App_InputTask

UsbTask
  -> App_UsbTask

BluetoothTask
  -> App_BluetoothTask

OledTask
  -> App_OledTask

RgbTask
  -> App_RgbTask
```

任务之间不直接调用彼此的业务函数，而是通过 Common 层的话题通信。

## 8. 上电启动流程

```text
HAL_Init
  ↓
系统时钟初始化
  ↓
GPIO/DMA/ADC/UART/TIM初始化
  ↓
ADC校准和摇杆DMA启动
  ↓
摇杆中位、LT/RT释放值采样
  ↓
FreeRTOS对象和任务初始化
  ↓
启动调度器
  ↓
各任务并行运行
```

上电校准期间需要：

```text
两个摇杆保持居中
LT、RT保持释放
```

## 9. 工程当前定位

当前工程已经不是单纯的 LED 测试工程，而是一个完整的手柄输入、显示和输出框架：

```text
硬件输入
  -> Bsp采集和处理
  -> InputTask发布话题
  -> USB/OLED/蓝牙节点分别订阅
  -> 输出到电脑、OLED或蓝牙设备
```

这种结构的优点是：

- 输入和输出解耦；
- USB、蓝牙、OLED可以独立开发；
- 后续可以增加控制节点；
- 可以将同一份输入数据发送给多个设备；
- 不需要在各个任务之间直接传递全局变量。

## 10. 后续可扩展方向

后续如果继续扩展，可以增加：

```text
控制节点
模式状态节点
蓝牙接收节点
电池电量节点
电源管理节点
```

推荐继续遵循：

```text
Bsp只负责硬件
App只负责业务
Common只负责消息通信
FreeRTOS只负责任务调度
```
