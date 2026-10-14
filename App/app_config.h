#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* 输入节点扫描周期，单位：ms */
#define APP_INPUT_SCAN_PERIOD_MS 10U

/* 按键状态连续稳定2次后确认，等效消抖时间约为20 ms */
#define APP_KEY_DEBOUNCE_SCAN_COUNT 2U

/* OLED节点检查按键和摇杆消息的周期，单位：ms */
#define APP_OLED_TASK_PERIOD_MS 10U

/* OLED界面刷新周期，单位：ms */
#define APP_OLED_REFRESH_PERIOD_MS 50U

/* USB HID报告发送周期，单位：ms */
#define APP_USB_TASK_PERIOD_MS 10U

/* 蓝牙节点检查按键消息的周期，单位：ms */
#define APP_BLUETOOTH_TASK_PERIOD_MS 5U

/* PC13状态灯翻转周期，单位：ms */
#define APP_RGB_BLINK_PERIOD_MS 500U

#endif
