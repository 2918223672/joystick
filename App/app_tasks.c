#include "app_tasks.h"

#include "cmsis_os.h"

#include "app_config.h"
#include "app_bus.h"
#include "bsp_led.h"
#include "bsp_key.h"
#include "bsp_oled.h"
#include "bsp_uart.h"
#include "usb_device.h"
#include "usbd_hid.h"

#include <string.h>

typedef enum
{
    APP_COMM_MODE_BLUETOOTH = 0,
    APP_COMM_MODE_USB
} AppCommunicationMode_t;

static AppCommunicationMode_t App_GetCommunicationMode(void)
{
    if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED)
    {
        return APP_COMM_MODE_USB;
    }

    return APP_COMM_MODE_BLUETOOTH;
}

/* 将12位ADC值映射为HID使用的有符号8位轴值。 */
static int8_t App_AdcToHidAxis(uint16_t value)
{
    int32_t mapped_value;

    mapped_value = ((int32_t)value * 254) / 4095 - 127;

    if (mapped_value < -127)
    {
        mapped_value = -127;
    }
    else if (mapped_value > 127)
    {
        mapped_value = 127;
    }

    return (int8_t)mapped_value;
}

/* 将已经归一化的LT/RT模拟量映射为HID的0~127触发值。 */
static uint8_t App_AdcToHidTrigger(uint16_t value)
{
    return (uint8_t)(((uint32_t)value * 127U) / 4095U);
}

/*
 * 将按键事件转换为蓝牙包：
 * 0x55 + ASCII按键名称 + 0xAA。
 */
static uint8_t App_BuildBluetoothPacket(const KeyData_t *key,
                                         uint8_t *packet,
                                         uint8_t *packet_length)
{
    const char *key_name;
    uint8_t key_name_length;

    if (key == 0 || packet == 0 || packet_length == 0 || key->pressed == 0U)
    {
        return 0U;
    }

    key_name = 0;
    key_name_length = 0U;

    switch ((BspKeyId_t)key->key_id)
    {
        case BSP_KEY_UP:
            key_name = "UP";
            key_name_length = 2U;
            break;

        case BSP_KEY_DOWN:
            key_name = "DOWN";
            key_name_length = 4U;
            break;

        case BSP_KEY_RIGHT:
            key_name = "RIGHT";
            key_name_length = 5U;
            break;

        case BSP_KEY_LEFT:
            key_name = "LEFT";
            key_name_length = 4U;
            break;

        case BSP_KEY_Y:
            key_name = "Y";
            key_name_length = 1U;
            break;

        case BSP_KEY_B:
            key_name = "B";
            key_name_length = 1U;
            break;

        case BSP_KEY_X:
            key_name = "X";
            key_name_length = 1U;
            break;

        case BSP_KEY_A:
            key_name = "A";
            key_name_length = 1U;
            break;

        default:
            return 0U;
    }

    packet[0] = 0x55U;
    memcpy(&packet[1], key_name, key_name_length);
    packet[key_name_length + 1U] = 0xAAU;
    *packet_length = key_name_length + 2U;

    return 1U;
}

uint8_t App_TasksInit(void)
{
    return App_BusInit();
}

void App_RgbTask(void *argument)
{
    (void)argument;

    for (;;)
    {
        Bsp_LedToggle();
        osDelay(APP_RGB_BLINK_PERIOD_MS);
    }
}

void App_UsbTask(void *argument)
{
    KeyData_t received_key;
    JoystickData_t received_joystick;
    JoystickData_t joystick_data;
    uint16_t key_mask;
    uint8_t report[8];

    (void)argument;

    joystick_data.lx = 2048U;
    joystick_data.ly = 2048U;
    joystick_data.rx = 2048U;
    joystick_data.ry = 2048U;
    joystick_data.lt = 0U;
    joystick_data.rt = 0U;
    key_mask = 0U;

    /*
     * USB初始化放在USB任务中，只初始化一次。
     * 这样输入任务只负责采集，USB任务只负责输出。
     */
    MX_USB_DEVICE_Init();

    for (;;)
    {
        while (App_BusReceiveUsbKey(&received_key) != 0U)
        {
            if (received_key.key_id < 16U)
            {
                if (received_key.pressed != 0U)
                {
                    key_mask |= (uint16_t)(1U << received_key.key_id);
                }
                else
                {
                    key_mask &= (uint16_t)~(uint16_t)(1U << received_key.key_id);
                }
            }
        }

        while (App_BusReceiveUsbJoystick(&received_joystick) != 0U)
        {
            joystick_data = received_joystick;
        }

        report[0] = (uint8_t)App_AdcToHidAxis(joystick_data.lx);
        report[1] = (uint8_t)App_AdcToHidAxis(joystick_data.ly);
        report[2] = (uint8_t)App_AdcToHidAxis(joystick_data.rx);
        report[3] = (uint8_t)App_AdcToHidAxis(joystick_data.ry);
        report[4] = App_AdcToHidTrigger(joystick_data.lt);
        report[5] = App_AdcToHidTrigger(joystick_data.rt);
        report[6] = (uint8_t)(key_mask & 0x00FFU);
        report[7] = (uint8_t)((key_mask >> 8) & 0x00FFU);

        if (App_GetCommunicationMode() == APP_COMM_MODE_USB)
        {
            (void)USBD_HID_SendReport(&hUsbDeviceFS, report, sizeof(report));
        }

        osDelay(APP_USB_TASK_PERIOD_MS);
    }
}

void App_BluetoothTask(void *argument)
{
    KeyData_t received_key;
    uint8_t packet[8];
    uint8_t packet_length;

    (void)argument;

    for (;;)
    {
        while (App_BusReceiveBluetoothKey(&received_key) != 0U)
        {
            if (App_GetCommunicationMode() == APP_COMM_MODE_BLUETOOTH &&
                App_BuildBluetoothPacket(&received_key, packet, &packet_length) != 0U)
            {
                (void)Bsp_UartSend(packet, packet_length, 10U);
            }
        }

        osDelay(APP_BLUETOOTH_TASK_PERIOD_MS);
    }
}

void App_OledTask(void *argument)
{
    JoystickData_t received_joystick;
    uint16_t left_x = 2048U;
    uint16_t left_y = 2048U;
    uint16_t right_x = 2048U;
    uint16_t right_y = 2048U;
    uint16_t left_trigger = 0U;
    uint16_t right_trigger = 0U;
    uint32_t last_refresh_tick;

    (void)argument;

    if (Bsp_OledInit() == 0U)
    {
        for (;;)
        {
            osDelay(1000U);
        }
    }

    Bsp_OledShowJoystick(left_x,
                         left_y,
                         right_x,
                         right_y,
                         left_trigger,
                         right_trigger);
    last_refresh_tick = osKernelGetTickCount();

    for (;;)
    {
        /*
         * 一次循环取出所有积压消息，只保留最新摇杆值，
         * 避免OLED逐帧显示已经过期的位置。
         */
        while (App_BusReceiveOledJoystick(&received_joystick) != 0U)
        {
            left_x = received_joystick.lx;
            left_y = received_joystick.ly;
            right_x = received_joystick.rx;
            right_y = received_joystick.ry;
            left_trigger = received_joystick.lt;
            right_trigger = received_joystick.rt;
        }

        if ((osKernelGetTickCount() - last_refresh_tick) >= APP_OLED_REFRESH_PERIOD_MS)
        {
            Bsp_OledShowJoystick(left_x,
                                 left_y,
                                 right_x,
                                 right_y,
                                 left_trigger,
                                 right_trigger);
            last_refresh_tick = osKernelGetTickCount();
        }

        osDelay(APP_OLED_TASK_PERIOD_MS);
    }
}
