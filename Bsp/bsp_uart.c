#include "bsp_uart.h"
#include "usart.h"

/* 阻塞发送 */
BspUartStatus_t Bsp_UartSend(const uint8_t *data, uint16_t length, uint32_t timeout)
{
    HAL_StatusTypeDef status;

    if (data == 0 || length == 0)
    {
        return BSP_UART_ERROR;
    }

    status = HAL_UART_Transmit(&huart1, (uint8_t *)data, length, timeout);

    if (status == HAL_OK)
    {
        return BSP_UART_OK;
    }

    if (status == HAL_BUSY)
    {
        return BSP_UART_BUSY;
    }

    return BSP_UART_ERROR;
}

/* DMA发送 */
BspUartStatus_t Bsp_UartSendDma(const uint8_t *data, uint16_t length)
{
    HAL_StatusTypeDef status;

    if (data == 0 || length == 0)
    {
        return BSP_UART_ERROR;
    }

    status = HAL_UART_Transmit_DMA(&huart1, (uint8_t *)data, length);

    if (status == HAL_OK)
    {
        return BSP_UART_OK;
    }

    if (status == HAL_BUSY)
    {
        return BSP_UART_BUSY;
    }

    return BSP_UART_ERROR;
}

/* DMA接收固定长度数据 */
BspUartStatus_t Bsp_UartReceiveDma(uint8_t *buffer, uint16_t length)
{
    HAL_StatusTypeDef status;

    if (buffer == 0 || length == 0)
    {
        return BSP_UART_ERROR;
    }

    status = HAL_UART_Receive_DMA(&huart1, buffer, length);

    if (status == HAL_OK)
    {
        return BSP_UART_OK;
    }

    if (status == HAL_BUSY)
    {
        return BSP_UART_BUSY;
    }

    return BSP_UART_ERROR;
}

/* 停止DMA接收 */
BspUartStatus_t Bsp_UartStopReceive(void)
{
    if (HAL_UART_DMAStop(&huart1) == HAL_OK)
    {
        return BSP_UART_OK;
    }

    return BSP_UART_ERROR;
}
