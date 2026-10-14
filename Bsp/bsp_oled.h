#ifndef BSP_OLED_H
#define BSP_OLED_H

#include <stdint.h>

/* 初始化SSD1306 128x64 OLED */
uint8_t Bsp_OledInit(void);

/* 清空OLED显存 */
void Bsp_OledClear(void);

/* 在OLED第一行显示ASCII字符串 */
void Bsp_OledShowString(const char *text);

/*
 * 显示双摇杆和LT/RT扳机状态。
 * 摇杆轴和扳机数据范围均为0~4095。
 */
void Bsp_OledShowJoystick(uint16_t left_x,
                          uint16_t left_y,
                          uint16_t right_x,
                          uint16_t right_y,
                          uint16_t left_trigger,
                          uint16_t right_trigger);

#endif
