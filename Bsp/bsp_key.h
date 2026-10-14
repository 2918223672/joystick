#ifndef BSP_KEY_H
#define BSP_KEY_H

#include <stdint.h>

/* 手柄按键编号 */
typedef enum
{
    BSP_KEY_R_STICK = 0,  /* 右摇杆按键 */
    BSP_KEY_L_STICK = 1,  /* 左摇杆按键 */

    BSP_KEY_RIGHT   = 2,  /* 方向右键 */
    BSP_KEY_DOWN    = 3,  /* 方向下键 */
    BSP_KEY_LEFT    = 4,  /* 方向左键 */
    BSP_KEY_UP      = 5,  /* 方向上键 */

    BSP_KEY_RB      = 6,  /* RB肩键 */
    BSP_KEY_LB      = 7,  /* LB肩键 */

    BSP_KEY_HOME    = 8,  /* HOME键 */
    BSP_KEY_MENU    = 9,  /* MENU键 */
    BSP_KEY_VIEW    = 10, /* VIEW键 */

    BSP_KEY_Y       = 11, /* Y键 */
    BSP_KEY_B       = 12, /* B键 */
    BSP_KEY_A       = 13, /* A键 */
    BSP_KEY_X       = 14, /* X键 */

    BSP_KEY_COUNT   = 15  /* 按键总数，不代表实际按键 */
} BspKeyId_t;

/*
 * 读取按键逻辑状态
 *
 * 返回值：
 * 1：按键按下
 * 0：按键未按下
 */
uint8_t Bsp_KeyIsPressed(BspKeyId_t key);

/* 获取按键显示名称，例如"BSP_KEY_A"对应"A" */
const char *Bsp_KeyGetName(BspKeyId_t key);

#endif
