#include "bsp_led.h"

/*
 * 当前LED连接在PC13，Blue Pill板载LED通常为低电平点亮：
 * RESET -> 点亮
 * SET   -> 熄灭
 */
void Bsp_LedOn(void)
{
    HAL_GPIO_WritePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin, GPIO_PIN_RESET);
}

void Bsp_LedOff(void)
{
    HAL_GPIO_WritePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin, GPIO_PIN_SET);
}

void Bsp_LedToggle(void)
{
    HAL_GPIO_TogglePin(LED_STATUS_GPIO_Port, LED_STATUS_Pin);
}
