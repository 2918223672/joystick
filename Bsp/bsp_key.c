#include "bsp_key.h"
#include "main.h"

/* 按键硬件映射表 */
typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} BspKeyConfig_t;

static const BspKeyConfig_t key_config[BSP_KEY_COUNT] =
{
    [BSP_KEY_R_STICK] = {KEY_R_STICK_GPIO_Port, KEY_R_STICK_Pin},
    [BSP_KEY_L_STICK] = {KEY_L_STICK_GPIO_Port, KEY_L_STICK_Pin},

    [BSP_KEY_RIGHT] = {KEY_RIGHT_GPIO_Port, KEY_RIGHT_Pin},
    [BSP_KEY_DOWN]  = {KEY_DOWN_GPIO_Port, KEY_DOWN_Pin},
    [BSP_KEY_LEFT]  = {KEY_LEFT_GPIO_Port, KEY_LEFT_Pin},
    [BSP_KEY_UP]    = {KEY_UP_GPIO_Port, KEY_UP_Pin},

    [BSP_KEY_RB] = {KEY_RB_GPIO_Port, KEY_RB_Pin},
    [BSP_KEY_LB] = {KEY_LB_GPIO_Port, KEY_LB_Pin},

    [BSP_KEY_HOME] = {KEY_HOME_GPIO_Port, KEY_HOME_Pin},
    [BSP_KEY_MENU] = {KEY_MENU_GPIO_Port, KEY_MENU_Pin},
    [BSP_KEY_VIEW] = {KEY_VIEW_GPIO_Port, KEY_VIEW_Pin},

    [BSP_KEY_Y] = {KEY_Y_GPIO_Port, KEY_Y_Pin},
    [BSP_KEY_B] = {KEY_B_GPIO_Port, KEY_B_Pin},
    [BSP_KEY_A] = {KEY_A_GPIO_Port, KEY_A_Pin},
    [BSP_KEY_X] = {KEY_X_GPIO_Port, KEY_X_Pin}
};

uint8_t Bsp_KeyIsPressed(BspKeyId_t key)
{
    if (key >= BSP_KEY_COUNT)
    {
        return 0;
    }
    if (HAL_GPIO_ReadPin(key_config[key].port, key_config[key].pin) == GPIO_PIN_RESET)
    {
        return 1;
    }

    return 0;
}

/* 返回按键名称，供OLED等显示模块使用 */
const char *Bsp_KeyGetName(BspKeyId_t key)
{
    static const char *const key_names[BSP_KEY_COUNT] =
    {
        [BSP_KEY_R_STICK] = "R_STICK",
        [BSP_KEY_L_STICK] = "L_STICK",
        [BSP_KEY_RIGHT] = "RIGHT",
        [BSP_KEY_DOWN] = "DOWN",
        [BSP_KEY_LEFT] = "LEFT",
        [BSP_KEY_UP] = "UP",
        [BSP_KEY_RB] = "RB",
        [BSP_KEY_LB] = "LB",
        [BSP_KEY_HOME] = "HOME",
        [BSP_KEY_MENU] = "MENU",
        [BSP_KEY_VIEW] = "VIEW",
        [BSP_KEY_Y] = "Y",
        [BSP_KEY_B] = "B",
        [BSP_KEY_A] = "A",
        [BSP_KEY_X] = "X"
    };

    if (key >= BSP_KEY_COUNT)
    {
        return "UNKNOWN";
    }

    return key_names[key];
}
