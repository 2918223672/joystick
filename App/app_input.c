#include "app_input.h"

#include "cmsis_os.h"

#include "app_config.h"
#include "app_bus.h"
#include "bsp_key.h"
#include "bsp_joystick.h"

/* 输入节点扫描全部按键，并只在状态变化时发布事件 */
void App_InputTask(void *argument)
{
    uint8_t raw_state[BSP_KEY_COUNT] = {0};
    uint8_t stable_state[BSP_KEY_COUNT] = {0};
    uint8_t debounce_count[BSP_KEY_COUNT] = {0};
    uint8_t current_state;
    KeyData_t key_event;
    BspJoystickFilteredData_t filtered_joystick;
    JoystickData_t joystick_data;
    BspKeyId_t key;

    (void)argument;

    /*
     * 记录上电时的原始状态和稳定状态，
     * 避免任务启动时产生虚假的按键事件。
     */
    for (key = BSP_KEY_R_STICK; key < BSP_KEY_COUNT; key++)
    {
        raw_state[key] = Bsp_KeyIsPressed(key);
        stable_state[key] = raw_state[key];
    }

    for (;;)
    {
        for (key = BSP_KEY_R_STICK; key < BSP_KEY_COUNT; key++)
        {
            current_state = Bsp_KeyIsPressed(key);

            /*
             * 原始状态发生变化，重新开始消抖计时。
             */
            if (current_state != raw_state[key])
            {
                raw_state[key] = current_state;
                debounce_count[key] = 0;
            }
            /*
             * 原始状态保持不变时，累计连续稳定次数。
             */
            else if (raw_state[key] != stable_state[key])
            {
                if (debounce_count[key] < APP_KEY_DEBOUNCE_SCAN_COUNT)
                {
                    debounce_count[key]++;
                }

                /*
                 * 连续稳定达到设定次数，确认按键状态改变，
                 * 然后通过TOPIC_KEY_EVENT发布一次事件。
                 */
                if (debounce_count[key] >= APP_KEY_DEBOUNCE_SCAN_COUNT)
                {
                    stable_state[key] = raw_state[key];
                    debounce_count[key] = 0;
                    key_event.key_id = (uint8_t)key;
                    key_event.pressed = stable_state[key];
                    App_BusPublishKey(&key_event);
                }
            }
        }

        /*
         * 摇杆由DMA持续采样，输入节点每10 ms读取一次平滑结果，
         * 再通过摇杆话题交给后续控制节点。
         */
        Bsp_JoystickReadFiltered(&filtered_joystick);
        joystick_data.lx = filtered_joystick.lx;
        joystick_data.ly = filtered_joystick.ly;
        joystick_data.rx = filtered_joystick.rx;
        joystick_data.ry = filtered_joystick.ry;
        joystick_data.lt = filtered_joystick.lt;
        joystick_data.rt = filtered_joystick.rt;
        App_BusPublishJoystick(&joystick_data);

        osDelay(APP_INPUT_SCAN_PERIOD_MS);
    }
}
