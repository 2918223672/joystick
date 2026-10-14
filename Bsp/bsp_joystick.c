#include "bsp_joystick.h"
#include "adc.h"

/*
 * ADC DMA数据缓存。
 * DMA会在后台持续修改该数组，因此使用volatile。
 */
static volatile uint16_t joystick_adc_buffer[BSP_JOYSTICK_COUNT];
static uint32_t joystick_filter_state[BSP_JOYSTICK_COUNT];
static uint16_t joystick_center[BSP_JOYSTICK_COUNT];
static uint8_t joystick_filter_initialized;

static uint16_t Bsp_JoystickApplyDeadzone(uint16_t value,
                                          uint16_t center)
{
    uint32_t difference;
    uint32_t span;
    uint32_t adjusted;

    if (value >= center)
    {
        difference = (uint32_t)value - center;

        if (difference <= BSP_JOYSTICK_DEADZONE_ADC)
        {
            return 2048U;
        }

        span = 4095U - center;
        if (span <= BSP_JOYSTICK_DEADZONE_ADC)
        {
            return 4095U;
        }

        adjusted = difference - BSP_JOYSTICK_DEADZONE_ADC;
        return (uint16_t)(2048U +
                          (adjusted * 2047U) /
                          (span - BSP_JOYSTICK_DEADZONE_ADC));
    }

    difference = (uint32_t)center - value;

    if (difference <= BSP_JOYSTICK_DEADZONE_ADC)
    {
        return 2048U;
    }

    span = center;
    if (span <= BSP_JOYSTICK_DEADZONE_ADC)
    {
        return 0U;
    }

    adjusted = difference - BSP_JOYSTICK_DEADZONE_ADC;
    return (uint16_t)(2048U -
                      (adjusted * 2048U) /
                      (span - BSP_JOYSTICK_DEADZONE_ADC));
}

static uint16_t Bsp_JoystickApplyTrigger(uint16_t value,
                                         uint16_t release_value,
                                         uint16_t full_value,
                                         uint8_t reverse)
{
    uint32_t active_value;
    uint32_t active_range;
    uint32_t trigger_value;

    if (reverse != 0U)
    {
        value = (uint16_t)(4095U - value);
        release_value = (uint16_t)(4095U - release_value);
    }

    if (full_value <= release_value)
    {
        return 0U;
    }

    if (value <= release_value)
    {
        return 0U;
    }

    if (value >= full_value)
    {
        return 4095U;
    }

    active_value = (uint32_t)value - release_value;
    if (active_value <= BSP_JOYSTICK_TRIGGER_DEADZONE_ADC)
    {
        return 0U;
    }

    active_range = (uint32_t)full_value - release_value;
    if (active_range <= BSP_JOYSTICK_TRIGGER_DEADZONE_ADC)
    {
        return 0U;
    }

    trigger_value = ((active_value - BSP_JOYSTICK_TRIGGER_DEADZONE_ADC) *
                     4095U) /
                    (active_range - BSP_JOYSTICK_TRIGGER_DEADZONE_ADC);

    if (trigger_value > 4095U)
    {
        trigger_value = 4095U;
    }

    return (uint16_t)trigger_value;
}

/* 校准ADC并启动DMA循环采样 */
uint8_t Bsp_JoystickInit(void)
{
    /*
     * STM32F1需要在启动ADC前校准。
     * MX_ADC1_Init()和MX_DMA_Init()必须已经执行。
     */
    if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
    {
        return 0;
    }

    /*
     * ADC扫描顺序与ADC1中的Rank一致：
     * Rank1: PA0 / LX
     * Rank2: PA1 / LY
     * Rank3: PA2 / RX
     * Rank4: PA3 / RY
     * Rank5: PA4 / LT
     * Rank6: PA5 / RT
     */
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)joystick_adc_buffer, BSP_JOYSTICK_COUNT) != HAL_OK)
    {
        return 0;
    }

    if (Bsp_JoystickCalibrate() == 0U)
    {
        return 0;
    }

    Bsp_JoystickFilterReset();
    return 1;
}

uint8_t Bsp_JoystickCalibrate(void)
{
    uint32_t sum[BSP_JOYSTICK_COUNT] = {0};
    uint16_t sample;
    uint8_t channel;

    /*
     * DMA已经启动后读取多组数据求平均，减少ADC噪声对中位值的影响。
     * 摇杆轴记录居中值，LT/RT记录释放值。
     * 上电时需要保持摇杆居中、LT/RT未按下。
     */
    for (sample = 0U; sample < BSP_JOYSTICK_CALIBRATION_SAMPLES; sample++)
    {
        for (channel = BSP_JOYSTICK_LX;
             channel < BSP_JOYSTICK_COUNT;
             channel++)
        {
            sum[channel] += joystick_adc_buffer[channel];
        }

        HAL_Delay(1U);
    }

    for (channel = BSP_JOYSTICK_LX;
         channel < BSP_JOYSTICK_COUNT;
         channel++)
    {
        joystick_center[channel] =
            (uint16_t)(sum[channel] / BSP_JOYSTICK_CALIBRATION_SAMPLES);
    }

    return 1U;
}

/* 读取指定通道的ADC原始值 */
uint16_t Bsp_JoystickReadRaw(BspJoystickChannel_t channel)
{
    if (channel >= BSP_JOYSTICK_COUNT)
    {
        return 0;
    }

    return joystick_adc_buffer[channel];
}

/* 读取全部摇杆数据 */
void Bsp_JoystickRead(BspJoystickRawData_t *data)
{
    if (data == 0)
    {
        return;
    }

    data->lx = joystick_adc_buffer[BSP_JOYSTICK_LX];
    data->ly = joystick_adc_buffer[BSP_JOYSTICK_LY];
    data->rx = joystick_adc_buffer[BSP_JOYSTICK_RX];
    data->ry = joystick_adc_buffer[BSP_JOYSTICK_RY];
    data->lt = joystick_adc_buffer[BSP_JOYSTICK_LT];
    data->rt = joystick_adc_buffer[BSP_JOYSTICK_RT];
}

/*
 * 一阶低通滤波：
 *
 * filtered = filtered + (raw - filtered) * 3 / 4
 *
 * 使用定点整数运算，不使用浮点数。相比原来的1/4更新速度更快，
 * 同时保留少量滤波效果。
 */
void Bsp_JoystickReadFiltered(BspJoystickFilteredData_t *data)
{
    BspJoystickRawData_t raw_data;
    uint16_t raw_values[BSP_JOYSTICK_COUNT];
    uint16_t filtered_values[BSP_JOYSTICK_COUNT];
    uint8_t index;

    if (data == 0)
    {
        return;
    }

    Bsp_JoystickRead(&raw_data);

    raw_values[BSP_JOYSTICK_LX] = raw_data.lx;
    raw_values[BSP_JOYSTICK_LY] = raw_data.ly;
    raw_values[BSP_JOYSTICK_RX] = raw_data.rx;
    raw_values[BSP_JOYSTICK_RY] = raw_data.ry;
    raw_values[BSP_JOYSTICK_LT] = raw_data.lt;
    raw_values[BSP_JOYSTICK_RT] = raw_data.rt;

    if (joystick_filter_initialized == 0U)
    {
        for (index = 0; index < BSP_JOYSTICK_COUNT; index++)
        {
            joystick_filter_state[index] = ((uint32_t)raw_values[index]) << 8;
        }

        joystick_filter_initialized = 1U;
    }
    else
    {
        for (index = 0; index < BSP_JOYSTICK_COUNT; index++)
        {
            int32_t difference;
            int32_t raw_value;
            int32_t filtered_value;

            raw_value = (int32_t)(((uint32_t)raw_values[index]) << 8);
            filtered_value = (int32_t)joystick_filter_state[index];
            difference = raw_value - filtered_value;

            /*
             * 差值很小时直接跟随，避免中心附近出现明显粘滞；
             * 差值较大时快速靠近目标值，降低操纵延迟。
             */
            if (difference < (8 * 256) && difference > -(8 * 256))
            {
                joystick_filter_state[index] = (uint32_t)raw_value;
            }
            else
            {
                joystick_filter_state[index] =
                    (uint32_t)(filtered_value + difference - (difference >> 2));
            }
        }
    }

    for (index = 0; index < BSP_JOYSTICK_COUNT; index++)
    {
        filtered_values[index] = (uint16_t)(joystick_filter_state[index] >> 8);
    }

    /*
     * 当前PCB实测结果：
     * 左摇杆物理Y轴接入的ADC通道表现为逻辑X轴；
     * 右摇杆同样需要交换X/Y。
     */
#if BSP_JOYSTICK_SWAP_LEFT_XY
    data->lx = Bsp_JoystickApplyDeadzone(
        filtered_values[BSP_JOYSTICK_LY],
        joystick_center[BSP_JOYSTICK_LY]);
    data->ly = Bsp_JoystickApplyDeadzone(
        filtered_values[BSP_JOYSTICK_LX],
        joystick_center[BSP_JOYSTICK_LX]);
#else
    data->lx = Bsp_JoystickApplyDeadzone(
        filtered_values[BSP_JOYSTICK_LX],
        joystick_center[BSP_JOYSTICK_LX]);
    data->ly = Bsp_JoystickApplyDeadzone(
        filtered_values[BSP_JOYSTICK_LY],
        joystick_center[BSP_JOYSTICK_LY]);
#endif

#if BSP_JOYSTICK_SWAP_RIGHT_XY
    data->rx = Bsp_JoystickApplyDeadzone(
        filtered_values[BSP_JOYSTICK_RY],
        joystick_center[BSP_JOYSTICK_RY]);
    data->ry = Bsp_JoystickApplyDeadzone(
        filtered_values[BSP_JOYSTICK_RX],
        joystick_center[BSP_JOYSTICK_RX]);
#else
    data->rx = Bsp_JoystickApplyDeadzone(
        filtered_values[BSP_JOYSTICK_RX],
        joystick_center[BSP_JOYSTICK_RX]);
    data->ry = Bsp_JoystickApplyDeadzone(
        filtered_values[BSP_JOYSTICK_RY],
        joystick_center[BSP_JOYSTICK_RY]);
#endif
    data->lt = Bsp_JoystickApplyTrigger(
        filtered_values[BSP_JOYSTICK_LT],
        joystick_center[BSP_JOYSTICK_LT],
        BSP_JOYSTICK_LT_FULL_ADC,
        BSP_JOYSTICK_REVERSE_LT);
    data->rt = Bsp_JoystickApplyTrigger(
        filtered_values[BSP_JOYSTICK_RT],
        joystick_center[BSP_JOYSTICK_RT],
        BSP_JOYSTICK_RT_FULL_ADC,
        BSP_JOYSTICK_REVERSE_RT);
}

void Bsp_JoystickFilterReset(void)
{
    uint8_t index;

    for (index = 0; index < BSP_JOYSTICK_COUNT; index++)
    {
        joystick_filter_state[index] = 0U;
    }

    joystick_filter_initialized = 0U;
}
