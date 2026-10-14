#ifndef COMMON_NODE_H
#define COMMON_NODE_H

#include "common_types.h"

/* 节点名称最大长度，不包含字符串结束符 */
#define COMMON_NODE_NAME_LEN 16

/* 系统中的功能节点编号 */
typedef enum
{
    NODE_NONE = 0, /* 无效节点 */
    NODE_INPUT,    /* 输入节点：读取摇杆、按键等输入设备 */
    NODE_CONTROL,  /* 控制节点：处理输入数据并生成控制指令 */
    NODE_OUTPUT,   /* 输出节点：向蓝牙、串口等外部设备发送数据 */
    NODE_OLED,     /* OLED节点：显示按键和系统状态 */
    NODE_USB,      /* USB HID节点：向电脑发送游戏手柄报告 */
    NODE_BLUETOOTH,/* 蓝牙节点：通过USART1发送自定义数据包 */
    NODE_COUNT      /* 节点总数，不作为实际节点使用 */
} NodeId_t;

/* 一个功能节点的实例 */
typedef struct
{
    NodeId_t id;                         /* 节点编号 */
    char name[COMMON_NODE_NAME_LEN + 1]; /* 节点名称 */
    uint8_t registered;                  /* 节点是否已经注册 */
} CommonNode_t;

#endif
