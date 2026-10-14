#ifndef BSP_UART_H
#define BSP_UART_H

#include <stdint.h>

/* UART操作结果 */
typedef enum
{
    BSP_UART_OK = 0, /* 操作成功 */
    BSP_UART_ERROR,  /* 参数或硬件错误 */
    BSP_UART_BUSY    /* UART正在执行其他操作 */
} BspUartStatus_t;

/* 阻塞发送，直到发送完成或超时 */
BspUartStatus_t Bsp_UartSend(const uint8_t *data, uint16_t length, uint32_t timeout);

/*
 * DMA发送。
 * DMA发送完成前，data指向的缓冲区不能被修改。
 */
BspUartStatus_t Bsp_UartSendDma(const uint8_t *data, uint16_t length);

/*
 * DMA接收固定长度数据。
 * 当前USART1的DMA接收模式为DMA_NORMAL。
 */
BspUartStatus_t Bsp_UartReceiveDma(uint8_t *buffer, uint16_t length);

/* 停止当前UART的DMA接收 */
BspUartStatus_t Bsp_UartStopReceive(void);

#endif
