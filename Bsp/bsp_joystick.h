#ifndef BSP_JOYSTICK_H
#define BSP_JOYSTICK_H

#include <stdint.h>

/*
 * 根据当前PCB实测方向，ADC扫描到的两个轴与逻辑X/Y轴需要交换。
 * 如果后续更换摇杆或重新布线，只需要修改这两个宏。
 */
#define BSP_JOYSTICK_SWAP_LEFT_XY   1U
#define BSP_JOYSTICK_SWAP_RIGHT_XY  1U

/* 上电校准时的采样次数，校准期间两个摇杆需要保持居中。 */
#define BSP_JOYSTICK_CALIBRATION_SAMPLES 32U

/* 摇杆死区，单位为ADC计数，120约占满量程的2.9%。 */
#define BSP_JOYSTICK_DEADZONE_ADC        120U

/* 扳机释放端死区，单位为ADC计数。 */
#define BSP_JOYSTICK_TRIGGER_DEADZONE_ADC 80U

/*
 * 扳机逻辑满按端点，单位为经过方向反转后的ADC计数。
 * 模拟扳机的机械满按值通常达不到4095，因此使用独立端点映射。
 * 如果实测满按值不同，只需要调整对应宏。
 */
#define BSP_JOYSTICK_LT_FULL_ADC          3600U
#define BSP_JOYSTICK_RT_FULL_ADC          3600U

/* 如果某个扳机按下时ADC值反而减小，将对应宏改为1。 */
#define BSP_JOYSTICK_REVERSE_LT          1U
#define BSP_JOYSTICK_REVERSE_RT          1U

/* 摇杆ADC通道编号 */
typedef enum
{
    BSP_JOYSTICK_LX = 0, /* 左摇杆X轴 */
    BSP_JOYSTICK_LY = 1, /* 左摇杆Y轴 */
    BSP_JOYSTICK_RX = 2, /* 右摇杆X轴 */
    BSP_JOYSTICK_RY = 3, /* 右摇杆Y轴 */
    BSP_JOYSTICK_LT = 4, /* 左扳机 */
    BSP_JOYSTICK_RT = 5, /* 右扳机 */

    BSP_JOYSTICK_COUNT = 6 /* ADC通道总数 */
} BspJoystickChannel_t;

/* 摇杆原始ADC数据 */
typedef struct
{
    uint16_t lx; /* 左摇杆X轴，范围约0~4095 */
    uint16_t ly; /* 左摇杆Y轴，范围约0~4095 */
    uint16_t rx; /* 右摇杆X轴，范围约0~4095 */
    uint16_t ry; /* 右摇杆Y轴，范围约0~4095 */
    uint16_t lt; /* 左扳机，范围约0~4095 */
    uint16_t rt; /* 右扳机，范围约0~4095 */
} BspJoystickRawData_t;

/*
 * 经过一阶低通、校准和死区处理并完成逻辑轴映射后的摇杆数据。
 * 四个摇杆轴会重新归一化到0~4095，中位统一为2048。
 */
typedef struct
{
    uint16_t lx; /* 左摇杆逻辑X轴 */
    uint16_t ly; /* 左摇杆逻辑Y轴 */
    uint16_t rx; /* 右摇杆逻辑X轴 */
    uint16_t ry; /* 右摇杆逻辑Y轴 */
    uint16_t lt; /* 左扳机 */
    uint16_t rt; /* 右扳机 */
} BspJoystickFilteredData_t;

/* 初始化摇杆ADC和DMA采样 */
uint8_t Bsp_JoystickInit(void);

/* 重新校准四个摇杆轴的中位值，执行期间摇杆必须保持居中 */
uint8_t Bsp_JoystickCalibrate(void);

/* 读取指定ADC通道的原始值 */
uint16_t Bsp_JoystickReadRaw(BspJoystickChannel_t channel);

/* 一次读取全部摇杆数据 */
void Bsp_JoystickRead(BspJoystickRawData_t *data);

/* 读取经过一阶低通滤波后的数据 */
void Bsp_JoystickReadFiltered(BspJoystickFilteredData_t *data);

/* 清除滤波器状态，下次读取时重新建立初始值 */
void Bsp_JoystickFilterReset(void);

#endif
