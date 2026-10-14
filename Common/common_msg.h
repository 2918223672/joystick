#ifndef COMMON_MSG_H
#define COMMON_MSG_H

#include "common_types.h"

/* 话题名称统一定义，避免不同节点手写字符串导致名称不一致 */
#define TOPIC_TEST             "test"
#define TOPIC_JOYSTICK_DATA    "joystick_data"
#define TOPIC_KEY_EVENT        "key_event"
#define TOPIC_CONTROL_CMD      "control_cmd"
#define TOPIC_LED_CMD          "led_cmd"

/* 摇杆数据，已经由输入节点完成基础平滑 */
typedef struct
{
    uint16_t lx;     /* 左摇杆X轴，范围约0~4095 */
    uint16_t ly;     /* 左摇杆Y轴，范围约0~4095 */
    uint16_t rx;     /* 右摇杆X轴，范围约0~4095 */
    uint16_t ry;     /* 右摇杆Y轴，范围约0~4095 */
    uint16_t lt;     /* 左扳机，范围约0~4095 */
    uint16_t rt;     /* 右扳机，范围约0~4095 */
} JoystickData_t;

/* 按键数据 */
typedef struct
{
    uint8_t key_id;  /* 按键编号 */
    uint8_t pressed; /* 按键状态：1表示按下，0表示释放 */
} KeyData_t;

/* 控制指令数据 */
typedef struct
{
    int16_t x;       /* X方向控制量 */
    int16_t y;       /* Y方向控制量 */
    uint8_t mode;    /* 控制模式 */
} ControlCmdData_t;

/* LED控制数据 */
typedef struct
{
    uint8_t state; /* LED状态：0表示关闭，1表示打开 */
} LedCmdData_t;

#endif
