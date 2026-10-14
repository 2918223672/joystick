#include "bsp_oled.h"

#include "main.h"

#define BSP_OLED_I2C_ADDRESS      0x78U
#define BSP_OLED_I2C_DELAY_COUNT  10U
#define BSP_OLED_WIDTH            128U
#define BSP_OLED_PAGE_COUNT       8U
#define BSP_OLED_FRAME_LEFT_X     10U
#define BSP_OLED_FRAME_RIGHT_X    74U
#define BSP_OLED_FRAME_TOP_Y      20U
#define BSP_OLED_FRAME_WIDTH      44U
#define BSP_OLED_FRAME_HEIGHT     40U
#define BSP_OLED_DOT_RADIUS       2U
#define BSP_OLED_TRIGGER_BAR_X    24U
#define BSP_OLED_TRIGGER_BAR_Y    0U
#define BSP_OLED_TRIGGER_BAR_WIDTH 80U
#define BSP_OLED_TRIGGER_BAR_HEIGHT 8U

static uint8_t oled_buffer[BSP_OLED_WIDTH * BSP_OLED_PAGE_COUNT];
static uint8_t oled_display_valid;
static uint8_t oled_last_left_dot_x;
static uint8_t oled_last_left_dot_y;
static uint8_t oled_last_right_dot_x;
static uint8_t oled_last_right_dot_y;
static uint16_t oled_last_trigger_position;
static uint16_t oled_last_left_trigger;
static uint16_t oled_last_right_trigger;

static const uint8_t font_upper[26][5] =
{
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, {0x7F, 0x49, 0x49, 0x49, 0x36},
    {0x3E, 0x41, 0x41, 0x41, 0x22}, {0x7F, 0x41, 0x41, 0x22, 0x1C},
    {0x7F, 0x49, 0x49, 0x49, 0x41}, {0x7F, 0x09, 0x09, 0x09, 0x01},
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, {0x7F, 0x08, 0x08, 0x08, 0x7F},
    {0x00, 0x41, 0x7F, 0x41, 0x00}, {0x20, 0x40, 0x41, 0x3F, 0x01},
    {0x7F, 0x08, 0x14, 0x22, 0x41}, {0x7F, 0x40, 0x40, 0x40, 0x40},
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, {0x7F, 0x04, 0x08, 0x10, 0x7F},
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, {0x7F, 0x09, 0x09, 0x09, 0x06},
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, {0x7F, 0x09, 0x19, 0x29, 0x46},
    {0x46, 0x49, 0x49, 0x49, 0x31}, {0x01, 0x01, 0x7F, 0x01, 0x01},
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, {0x1F, 0x20, 0x40, 0x20, 0x1F},
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, {0x63, 0x14, 0x08, 0x14, 0x63},
    {0x07, 0x08, 0x70, 0x08, 0x07}, {0x61, 0x51, 0x49, 0x45, 0x43}
};

static const uint8_t font_digit[10][5] =
{
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, {0x00, 0x42, 0x7F, 0x40, 0x00},
    {0x42, 0x61, 0x51, 0x49, 0x46}, {0x21, 0x41, 0x45, 0x4B, 0x31},
    {0x18, 0x14, 0x12, 0x7F, 0x10}, {0x27, 0x45, 0x45, 0x45, 0x39},
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, {0x01, 0x71, 0x09, 0x05, 0x03},
    {0x36, 0x49, 0x49, 0x49, 0x36}, {0x06, 0x49, 0x49, 0x29, 0x1E}
};

static void Oled_Delay(void)
{
    volatile uint16_t count;

    for (count = 0; count < BSP_OLED_I2C_DELAY_COUNT; count++)
    {
        __NOP();
    }
}

static void Oled_SclWrite(GPIO_PinState state)
{
    if (state == GPIO_PIN_SET)
    {
        OLED_SCL_GPIO_Port->BSRR = OLED_SCL_Pin;
    }
    else
    {
        OLED_SCL_GPIO_Port->BRR = OLED_SCL_Pin;
    }

    Oled_Delay();
}

static void Oled_SdaWrite(GPIO_PinState state)
{
    if (state == GPIO_PIN_SET)
    {
        OLED_SDA_GPIO_Port->BSRR = OLED_SDA_Pin;
    }
    else
    {
        OLED_SDA_GPIO_Port->BRR = OLED_SDA_Pin;
    }

    Oled_Delay();
}

static void Oled_I2cStart(void)
{
    Oled_SdaWrite(GPIO_PIN_SET);
    Oled_SclWrite(GPIO_PIN_SET);
    Oled_SdaWrite(GPIO_PIN_RESET);
    Oled_SclWrite(GPIO_PIN_RESET);
}

static void Oled_I2cStop(void)
{
    Oled_SdaWrite(GPIO_PIN_RESET);
    Oled_SclWrite(GPIO_PIN_SET);
    Oled_SdaWrite(GPIO_PIN_SET);
}

static void Oled_I2cWriteByte(uint8_t data)
{
    uint8_t bit;

    for (bit = 0; bit < 8U; bit++)
    {
        Oled_SdaWrite((data & 0x80U) != 0U ? GPIO_PIN_SET : GPIO_PIN_RESET);
        Oled_SclWrite(GPIO_PIN_SET);
        Oled_SclWrite(GPIO_PIN_RESET);
        data <<= 1;
    }

    /* 释放SDA并提供ACK时钟，OLED的ACK内容这里不需要读取。 */
    Oled_SdaWrite(GPIO_PIN_SET);
    Oled_SclWrite(GPIO_PIN_SET);
    Oled_SclWrite(GPIO_PIN_RESET);
}

static void Oled_WriteCommand(uint8_t command)
{
    Oled_I2cStart();
    Oled_I2cWriteByte(BSP_OLED_I2C_ADDRESS);
    Oled_I2cWriteByte(0x00U);
    Oled_I2cWriteByte(command);
    Oled_I2cStop();
}

static void Oled_SetPosition(uint8_t page, uint8_t column)
{
    Oled_WriteCommand((uint8_t)(0xB0U | page));
    Oled_WriteCommand((uint8_t)(0x10U | ((column >> 4) & 0x0FU)));
    Oled_WriteCommand((uint8_t)(0x00U | (column & 0x0FU)));
}

static void Oled_WriteDataStart(void)
{
    Oled_I2cStart();
    Oled_I2cWriteByte(BSP_OLED_I2C_ADDRESS);
    Oled_I2cWriteByte(0x40U);
}

static void Oled_WriteDataStop(void)
{
    Oled_I2cStop();
}

static void Oled_BufferClear(void)
{
    uint16_t index;

    for (index = 0U; index < sizeof(oled_buffer); index++)
    {
        oled_buffer[index] = 0x00U;
    }
}

static void Oled_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    uint16_t buffer_index;
    uint8_t bit_mask;

    if (x >= BSP_OLED_WIDTH || y >= (BSP_OLED_PAGE_COUNT * 8U))
    {
        return;
    }

    buffer_index = (uint16_t)x + (uint16_t)(y / 8U) * BSP_OLED_WIDTH;
    bit_mask = (uint8_t)(1U << (y % 8U));

    if (color != 0U)
    {
        oled_buffer[buffer_index] |= bit_mask;
    }
    else
    {
        oled_buffer[buffer_index] &= (uint8_t)~bit_mask;
    }
}

static void Oled_DrawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    uint8_t index;
    uint8_t right = (uint8_t)(x + width - 1U);
    uint8_t bottom = (uint8_t)(y + height - 1U);

    for (index = x; index <= right; index++)
    {
        Oled_DrawPixel(index, y, 1U);
        Oled_DrawPixel(index, bottom, 1U);
    }

    for (index = y; index <= bottom; index++)
    {
        Oled_DrawPixel(x, index, 1U);
        Oled_DrawPixel(right, index, 1U);
    }
}

static void Oled_DrawFilledCircle(uint8_t center_x, uint8_t center_y, uint8_t radius)
{
    int16_t x;
    int16_t y;
    int16_t radius_square = (int16_t)radius * (int16_t)radius;

    for (y = -(int16_t)radius; y <= (int16_t)radius; y++)
    {
        for (x = -(int16_t)radius; x <= (int16_t)radius; x++)
        {
            if ((x * x + y * y) <= radius_square)
            {
                Oled_DrawPixel((uint8_t)((int16_t)center_x + x),
                               (uint8_t)((int16_t)center_y + y),
                               1U);
            }
        }
    }
}

static void Oled_DrawFilledRect(uint8_t x,
                                uint8_t y,
                                uint8_t width,
                                uint8_t height)
{
    uint16_t current_x;
    uint16_t current_y;

    for (current_y = y; current_y < (uint16_t)y + height; current_y++)
    {
        for (current_x = x; current_x < (uint16_t)x + width; current_x++)
        {
            Oled_DrawPixel((uint8_t)current_x, (uint8_t)current_y, 1U);
        }
    }
}

static uint8_t Oled_GetFontColumn(char character, uint8_t column)
{
    if (column >= 5U)
    {
        return 0x00U;
    }

    if (character >= 'A' && character <= 'Z')
    {
        return font_upper[(uint8_t)(character - 'A')][column];
    }

    if (character >= '0' && character <= '9')
    {
        return font_digit[(uint8_t)(character - '0')][column];
    }

    if (character == ':')
    {
        static const uint8_t colon[5] = {0x00, 0x36, 0x36, 0x00, 0x00};
        return colon[column];
    }

    if (character == '_')
    {
        static const uint8_t underscore[5] = {0x40, 0x40, 0x40, 0x40, 0x40};
        return underscore[column];
    }

    if (character == '-')
    {
        static const uint8_t dash[5] = {0x08, 0x08, 0x08, 0x08, 0x08};
        return dash[column];
    }

    return 0x00U;
}

static void Oled_DrawChar(uint8_t x, uint8_t y, char character)
{
    uint8_t column;
    uint8_t row;
    uint8_t font_column;

    for (column = 0U; column < 5U; column++)
    {
        font_column = Oled_GetFontColumn(character, column);
        for (row = 0U; row < 7U; row++)
        {
            if ((font_column & (uint8_t)(1U << row)) != 0U)
            {
                Oled_DrawPixel((uint8_t)(x + column), (uint8_t)(y + row), 1U);
            }
        }
    }
}

static void Oled_DrawString(uint8_t x, uint8_t y, const char *text)
{
    while (*text != '\0' && x <= (BSP_OLED_WIDTH - 6U))
    {
        Oled_DrawChar(x, y, *text);
        x = (uint8_t)(x + 6U);
        text++;
    }
}

static uint8_t Oled_MapAdcToPixel(uint16_t adc_value,
                                  uint8_t pixel_min,
                                  uint8_t pixel_max)
{
    uint32_t pixel_range;

    if (adc_value > 4095U)
    {
        adc_value = 4095U;
    }

    pixel_range = (uint32_t)pixel_max - pixel_min;
    return (uint8_t)(pixel_min + ((uint32_t)adc_value * pixel_range) / 4095U);
}

static void Oled_DrawJoystickFrame(uint8_t frame_x,
                                    uint16_t axis_x,
                                    uint16_t axis_y)
{
    uint8_t dot_min_x = (uint8_t)(frame_x + BSP_OLED_DOT_RADIUS);
    uint8_t dot_max_x = (uint8_t)(frame_x + BSP_OLED_FRAME_WIDTH - 1U -
                                  BSP_OLED_DOT_RADIUS);
    uint8_t dot_min_y = (uint8_t)(BSP_OLED_FRAME_TOP_Y + BSP_OLED_DOT_RADIUS);
    uint8_t dot_max_y = (uint8_t)(BSP_OLED_FRAME_TOP_Y +
                                  BSP_OLED_FRAME_HEIGHT - 1U -
                                  BSP_OLED_DOT_RADIUS);
    uint8_t dot_x = Oled_MapAdcToPixel(axis_x, dot_min_x, dot_max_x);
    uint8_t dot_y = Oled_MapAdcToPixel(axis_y, dot_min_y, dot_max_y);

    Oled_DrawRect(frame_x,
                  BSP_OLED_FRAME_TOP_Y,
                  BSP_OLED_FRAME_WIDTH,
                  BSP_OLED_FRAME_HEIGHT);
    Oled_DrawFilledCircle(dot_x, dot_y, BSP_OLED_DOT_RADIUS);
}

/*
 * 顶部扳机条：
 * - 初始位置在中点；
 * - LT越大，填充终点越向左；
 * - RT越大，填充终点越向右；
 * - LT满量程时没有填充；
 * - RT满量程时填满内部区域。
 */
static uint16_t Oled_GetTriggerBarPosition(uint16_t left_trigger,
                                           uint16_t right_trigger)
{
    int32_t position;

    position = 2048;
    position -= ((int32_t)left_trigger * 2048) / 4095;
    position += ((int32_t)right_trigger * 2047) / 4095;

    if (position < 0)
    {
        position = 0;
    }
    else if (position > 4095)
    {
        position = 4095;
    }

    return (uint16_t)position;
}

static void Oled_DrawTriggerBar(uint16_t left_trigger,
                                uint16_t right_trigger)
{
    uint16_t position;
    uint8_t inside_left;
    uint8_t inside_right;
    uint8_t fill_end;
    uint8_t fill_width;

    position = Oled_GetTriggerBarPosition(left_trigger, right_trigger);

    Oled_DrawRect(BSP_OLED_TRIGGER_BAR_X,
                  BSP_OLED_TRIGGER_BAR_Y,
                  BSP_OLED_TRIGGER_BAR_WIDTH,
                  BSP_OLED_TRIGGER_BAR_HEIGHT);

    inside_left = (uint8_t)(BSP_OLED_TRIGGER_BAR_X + 1U);
    inside_right = (uint8_t)(BSP_OLED_TRIGGER_BAR_X +
                             BSP_OLED_TRIGGER_BAR_WIDTH - 2U);

    if (position == 0)
    {
        return;
    }

    fill_end = (uint8_t)(inside_left +
                         ((uint32_t)(inside_right - inside_left) *
                          (uint32_t)position) / 4095U);
    fill_width = (uint8_t)(fill_end - inside_left + 1U);

    Oled_DrawFilledRect(inside_left,
                        (uint8_t)(BSP_OLED_TRIGGER_BAR_Y + 1U),
                        fill_width,
                        (uint8_t)(BSP_OLED_TRIGGER_BAR_HEIGHT - 2U));
}

static void Oled_Refresh(void)
{
    uint8_t page;
    uint16_t column;

    for (page = 0U; page < BSP_OLED_PAGE_COUNT; page++)
    {
        Oled_SetPosition(page, 0U);
        Oled_WriteDataStart();

        for (column = 0U; column < BSP_OLED_WIDTH; column++)
        {
            Oled_I2cWriteByte(oled_buffer[(uint16_t)page * BSP_OLED_WIDTH + column]);
        }

        Oled_WriteDataStop();
    }
}

/*
 * 摇杆界面只使用以下区域：
 * page 0~1：按键名称和左右摇杆标题；
 * page 2~7：左右两个摇杆框。
 *
 * 显存每帧仍然先清空再重绘，因此这些区域可以覆盖上一帧数据，
 * 同时避免发送没有使用的中间列。
 */
static void Oled_RefreshRange(uint8_t page,
                              uint8_t column_start,
                              uint8_t column_end)
{
    uint16_t column;

    Oled_SetPosition(page, column_start);
    Oled_WriteDataStart();

    for (column = column_start; column <= column_end; column++)
    {
        Oled_I2cWriteByte(oled_buffer[(uint16_t)page * BSP_OLED_WIDTH + column]);
    }

    Oled_WriteDataStop();
}

static void Oled_RefreshJoystickUi(void)
{
    uint8_t page;

    Oled_RefreshRange(0U, 0U, BSP_OLED_WIDTH - 1U);
    Oled_RefreshRange(1U, 0U, BSP_OLED_WIDTH - 1U);

    for (page = 2U; page < BSP_OLED_PAGE_COUNT; page++)
    {
        Oled_RefreshRange(page,
                          BSP_OLED_FRAME_LEFT_X,
                          BSP_OLED_FRAME_LEFT_X + BSP_OLED_FRAME_WIDTH - 1U);
        Oled_RefreshRange(page,
                          BSP_OLED_FRAME_RIGHT_X,
                          BSP_OLED_FRAME_RIGHT_X + BSP_OLED_FRAME_WIDTH - 1U);
    }
}

uint8_t Bsp_OledInit(void)
{
    /* SSD1306上电后需要一小段稳定时间 */
    HAL_Delay(100U);

    Oled_SdaWrite(GPIO_PIN_SET);
    Oled_SclWrite(GPIO_PIN_SET);

    Oled_WriteCommand(0xAEU);
    Oled_WriteCommand(0x20U);
    /* 后续使用0xB0按页寻址，因此必须选择Page Addressing Mode */
    Oled_WriteCommand(0x02U);
    Oled_WriteCommand(0xB0U);
    Oled_WriteCommand(0xC8U);
    Oled_WriteCommand(0x00U);
    Oled_WriteCommand(0x10U);
    Oled_WriteCommand(0x40U);
    Oled_WriteCommand(0x81U);
    Oled_WriteCommand(0x7FU);
    Oled_WriteCommand(0xA1U);
    Oled_WriteCommand(0xA6U);
    Oled_WriteCommand(0xA8U);
    Oled_WriteCommand(0x3FU);
    Oled_WriteCommand(0xA4U);
    Oled_WriteCommand(0xD3U);
    Oled_WriteCommand(0x00U);
    Oled_WriteCommand(0xD5U);
    Oled_WriteCommand(0x80U);
    Oled_WriteCommand(0xD9U);
    Oled_WriteCommand(0xF1U);
    Oled_WriteCommand(0xDAU);
    Oled_WriteCommand(0x12U);
    Oled_WriteCommand(0xDBU);
    Oled_WriteCommand(0x40U);
    Oled_WriteCommand(0x8DU);
    Oled_WriteCommand(0x14U);
    Oled_WriteCommand(0xAFU);

    oled_display_valid = 0U;
    Bsp_OledClear();
    return 1U;
}

void Bsp_OledClear(void)
{
    Oled_BufferClear();
    Oled_Refresh();
    oled_display_valid = 0U;
}

void Bsp_OledShowString(const char *text)
{
    if (text == 0)
    {
        return;
    }

    Oled_BufferClear();
    Oled_DrawString(0U, 0U, text);
    Oled_Refresh();
    oled_display_valid = 0U;
}

void Bsp_OledShowJoystick(uint16_t left_x,
                          uint16_t left_y,
                          uint16_t right_x,
                          uint16_t right_y,
                          uint16_t left_trigger,
                          uint16_t right_trigger)
{
    uint8_t left_dot_min_x;
    uint8_t left_dot_max_x;
    uint8_t right_dot_min_x;
    uint8_t right_dot_max_x;
    uint8_t dot_min_y;
    uint8_t dot_max_y;
    uint8_t left_dot_x;
    uint8_t left_dot_y;
    uint8_t right_dot_x;
    uint8_t right_dot_y;
    uint16_t trigger_position;

    left_dot_min_x = (uint8_t)(BSP_OLED_FRAME_LEFT_X + BSP_OLED_DOT_RADIUS);
    left_dot_max_x = (uint8_t)(BSP_OLED_FRAME_LEFT_X +
                               BSP_OLED_FRAME_WIDTH - 1U -
                               BSP_OLED_DOT_RADIUS);
    right_dot_min_x = (uint8_t)(BSP_OLED_FRAME_RIGHT_X + BSP_OLED_DOT_RADIUS);
    right_dot_max_x = (uint8_t)(BSP_OLED_FRAME_RIGHT_X +
                                BSP_OLED_FRAME_WIDTH - 1U -
                                BSP_OLED_DOT_RADIUS);
    dot_min_y = (uint8_t)(BSP_OLED_FRAME_TOP_Y + BSP_OLED_DOT_RADIUS);
    dot_max_y = (uint8_t)(BSP_OLED_FRAME_TOP_Y +
                          BSP_OLED_FRAME_HEIGHT - 1U -
                          BSP_OLED_DOT_RADIUS);
    left_dot_x = Oled_MapAdcToPixel(left_x, left_dot_min_x, left_dot_max_x);
    left_dot_y = Oled_MapAdcToPixel(left_y, dot_min_y, dot_max_y);
    right_dot_x = Oled_MapAdcToPixel(right_x, right_dot_min_x, right_dot_max_x);
    right_dot_y = Oled_MapAdcToPixel(right_y, dot_min_y, dot_max_y);
    trigger_position = Oled_GetTriggerBarPosition(left_trigger, right_trigger);

    if (oled_display_valid != 0U &&
        oled_last_left_dot_x == left_dot_x &&
        oled_last_left_dot_y == left_dot_y &&
        oled_last_right_dot_x == right_dot_x &&
        oled_last_right_dot_y == right_dot_y &&
        oled_last_trigger_position == trigger_position &&
        oled_last_left_trigger == left_trigger &&
        oled_last_right_trigger == right_trigger)
    {
        return;
    }

    Oled_BufferClear();
    Oled_DrawTriggerBar(left_trigger, right_trigger);
    Oled_DrawString(28U, 11U, "L");
    Oled_DrawString(92U, 11U, "R");
    Oled_DrawJoystickFrame(BSP_OLED_FRAME_LEFT_X, left_x, left_y);
    Oled_DrawJoystickFrame(BSP_OLED_FRAME_RIGHT_X, right_x, right_y);
    Oled_RefreshJoystickUi();

    oled_last_left_dot_x = left_dot_x;
    oled_last_left_dot_y = left_dot_y;
    oled_last_right_dot_x = right_dot_x;
    oled_last_right_dot_y = right_dot_y;
    oled_last_trigger_position = trigger_position;
    oled_last_left_trigger = left_trigger;
    oled_last_right_trigger = right_trigger;
    oled_display_valid = 1U;
}
