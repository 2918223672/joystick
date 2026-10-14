#ifndef APP_BUS_H
#define APP_BUS_H

#include "common_msg.h"

/* 初始化输入、OLED、USB和蓝牙节点以及输入话题 */
uint8_t App_BusInit(void);

/* 发布一条按键事件 */
uint8_t App_BusPublishKey(const KeyData_t *data);

/* 发布一条经过平滑处理的摇杆数据 */
uint8_t App_BusPublishJoystick(const JoystickData_t *data);

/* OLED节点从摇杆话题中读取一条摇杆数据 */
uint8_t App_BusReceiveOledJoystick(JoystickData_t *data);

/* USB节点读取按键事件 */
uint8_t App_BusReceiveUsbKey(KeyData_t *data);

/* USB节点读取摇杆数据 */
uint8_t App_BusReceiveUsbJoystick(JoystickData_t *data);

/* 蓝牙节点读取按键事件 */
uint8_t App_BusReceiveBluetoothKey(KeyData_t *data);

#endif
