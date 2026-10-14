#ifndef APP_TASKS_H
#define APP_TASKS_H

#include <stdint.h>

/* 初始化App通信 */
uint8_t App_TasksInit(void);

/* RGB/状态灯任务：让PC13每500 ms翻转一次 */
void App_RgbTask(void *argument);

/* USB HID任务：向电脑发送游戏手柄报告 */
void App_UsbTask(void *argument);

/* 蓝牙任务：发送自定义按键数据包 */
void App_BluetoothTask(void *argument);

/* OLED任务：接收按键事件并显示最近按下的按键 */
void App_OledTask(void *argument);

#endif
