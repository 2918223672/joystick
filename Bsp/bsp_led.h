#ifndef BSP_LED_H
#define BSP_LED_H

#include "main.h"

/* 点亮状态指LED发光，不直接暴露PC13的电平逻辑 */
void Bsp_LedOn(void);

/* 熄灭LED */
void Bsp_LedOff(void);

/* 翻转LED当前状态 */
void Bsp_LedToggle(void);

#endif
