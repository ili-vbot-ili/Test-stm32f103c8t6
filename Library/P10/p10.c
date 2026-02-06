/**
 ******************************************************************************
 * @file           : p10.c
 * @brief          : P10 LED Matrix Module Implementation (HUB12 Interface)
 *                   Compatible with STM32F103C8T6
 ******************************************************************************
 */

#include "p10.h"
#include <string.h>

/* ============================================================================
 * Frame Buffer
 * ============================================================================ */

static uint8_t P10_Buffer[P10_BUFFER_SIZE];
static uint8_t P10_CurrentRow = 0;
static uint8_t P10_Brightness = 100;

/* ============================================================================
 * Font 5x7 (Basic ASCII 32-127)
 * ============================================================================ */

static const uint8_t Font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Space (32)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // ! (33)
    {0x00, 0x07, 0x00, 0x07, 0x00}, // " (34)
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // # (35)
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $ (36)
    {0x23, 0x13, 0x08, 0x64, 0x62}, // % (37)
    {0x36, 0x49, 0x55, 0x22, 0x50}, // & (38)
    {0x00, 0x05, 0x03, 0x00, 0x00}, // ' (39)
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // ( (40)
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // ) (41)
    {0x08, 0x2A, 0x1C, 0x2A, 0x08}, // * (42)
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // + (43)
    {0x00, 0x50, 0x30, 0x00, 0x00}, // , (44)
    {0x08, 0x08, 0x08, 0x08, 0x08}, // - (45)
    {0x00, 0x60, 0x60, 0x00, 0x00}, // . (46)
    {0x20, 0x10, 0x08, 0x04, 0x02}, // / (47)
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0 (48)
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1 (49)
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2 (50)
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3 (51)
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4 (52)
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5 (53)
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6 (54)
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7 (55)
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8 (56)
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9 (57)
    {0x00, 0x36, 0x36, 0x00, 0x00}, // : (58)
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ; (59)
    {0x00, 0x08, 0x14, 0x22, 0x41}, // < (60)
    {0x14, 0x14, 0x14, 0x14, 0x14}, // = (61)
    {0x41, 0x22, 0x14, 0x08, 0x00}, // > (62)
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ? (63)
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @ (64)
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A (65)
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B (66)
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C (67)
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D (68)
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E (69)
    {0x7F, 0x09, 0x09, 0x01, 0x01}, // F (70)
    {0x3E, 0x41, 0x41, 0x51, 0x32}, // G (71)
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H (72)
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I (73)
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J (74)
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K (75)
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L (76)
    {0x7F, 0x02, 0x04, 0x02, 0x7F}, // M (77)
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N (78)
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O (79)
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P (80)
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q (81)
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R (82)
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S (83)
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T (84)
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U (85)
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V (86)
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, // W (87)
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X (88)
    {0x03, 0x04, 0x78, 0x04, 0x03}, // Y (89)
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z (90)
    {0x00, 0x00, 0x7F, 0x41, 0x41}, // [ (91)
    {0x02, 0x04, 0x08, 0x10, 0x20}, // \ (92)
    {0x41, 0x41, 0x7F, 0x00, 0x00}, // ] (93)
    {0x04, 0x02, 0x01, 0x02, 0x04}, // ^ (94)
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _ (95)
    {0x00, 0x01, 0x02, 0x04, 0x00}, // ` (96)
    {0x20, 0x54, 0x54, 0x54, 0x78}, // a (97)
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // b (98)
    {0x38, 0x44, 0x44, 0x44, 0x20}, // c (99)
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // d (100)
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e (101)
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // f (102)
    {0x08, 0x14, 0x54, 0x54, 0x3C}, // g (103)
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // h (104)
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // i (105)
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // j (106)
    {0x00, 0x7F, 0x10, 0x28, 0x44}, // k (107)
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // l (108)
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // m (109)
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // n (110)
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o (111)
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // p (112)
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // q (113)
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // r (114)
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s (115)
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // t (116)
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // u (117)
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // v (118)
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // w (119)
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x (120)
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // y (121)
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // z (122)
    {0x00, 0x08, 0x36, 0x41, 0x00}, // { (123)
    {0x00, 0x00, 0x7F, 0x00, 0x00}, // | (124)
    {0x00, 0x41, 0x36, 0x08, 0x00}, // } (125)
    {0x08, 0x08, 0x2A, 0x1C, 0x08}, // -> (126)
    {0x08, 0x1C, 0x2A, 0x08, 0x08}, // <- (127)
};

/* ============================================================================
 * Private Functions
 * ============================================================================ */

/**
 * @brief  Delay microseconds (approximate)
 */
static void P10_DelayUs(uint32_t us)
{
    volatile uint32_t count = us * 8; // Approximate for 72MHz
    while(count--);
}

/**
 * @brief  Send byte via SPI (bit-bang or hardware)
 */
static void P10_SendByte(uint8_t data)
{
    for(int8_t i = 7; i >= 0; i--)
    {
        P10_CLK_LOW();
        
        if(data & (1 << i)) {
            P10_DATA_HIGH();
        } else {
            P10_DATA_LOW();
        }
        
        P10_CLK_HIGH();
    }
}

/**
 * @brief  Select row (0-3 for 1/4 scan)
 */
static void P10_SelectRow(uint8_t row)
{
    if(row & 0x01) {
        P10_A_HIGH();
    } else {
        P10_A_LOW();
    }
    
    if(row & 0x02) {
        P10_B_HIGH();
    } else {
        P10_B_LOW();
    }
}

/* ============================================================================
 * Public Functions
 * ============================================================================ */

void P10_Init(void)
{
    /* Enable GPIOB clock */
    P10_RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    
    /* Configure pins as output push-pull, 50MHz */
    /* PB10 (OE), PB11 (A), PB12 (B) - in CRH */
    /* Clear and set PB10 */
    P10_GPIOB->CRH &= ~(0x0F << 8);   // Clear PB10 config
    P10_GPIOB->CRH |= (0x03 << 8);    // Output 50MHz, Push-Pull
    
    /* Clear and set PB11 */
    P10_GPIOB->CRH &= ~(0x0F << 12);  // Clear PB11 config
    P10_GPIOB->CRH |= (0x03 << 12);   // Output 50MHz, Push-Pull
    
    /* Clear and set PB12 */
    P10_GPIOB->CRH &= ~(0x0F << 16);  // Clear PB12 config
    P10_GPIOB->CRH |= (0x03 << 16);   // Output 50MHz, Push-Pull
    
    /* PB13 (CLK), PB14 (LAT), PB15 (DATA) */
    /* Clear and set PB13 */
    P10_GPIOB->CRH &= ~(0x0F << 20);  // Clear PB13 config
    P10_GPIOB->CRH |= (0x03 << 20);   // Output 50MHz, Push-Pull
    
    /* Clear and set PB14 */
    P10_GPIOB->CRH &= ~(0x0F << 24);  // Clear PB14 config
    P10_GPIOB->CRH |= (0x03 << 24);   // Output 50MHz, Push-Pull
    
    /* Clear and set PB15 */
    P10_GPIOB->CRH &= ~(0x0F << 28);  // Clear PB15 config
    P10_GPIOB->CRH |= (0x03 << 28);   // Output 50MHz, Push-Pull
    
    /* Initial state */
    P10_OE_HIGH();  // Display off
    P10_LAT_LOW();
    P10_CLK_LOW();
    P10_A_LOW();
    P10_B_LOW();
    
    /* Clear buffer */
    P10_Clear();
    
    /* Enable display */
    P10_OE_LOW();
}

void P10_Clear(void)
{
    memset(P10_Buffer, 0xFF, P10_BUFFER_SIZE); // 0xFF = all LEDs off (inverted logic)
}

void P10_Fill(void)
{
    memset(P10_Buffer, 0x00, P10_BUFFER_SIZE); // 0x00 = all LEDs on (inverted logic)
}

void P10_SetPixel(uint16_t x, uint16_t y, uint8_t value)
{
    if(x >= P10_WIDTH || y >= P10_HEIGHT) {
        return;
    }
    
    /* Calculate byte and bit position */
    /* P10 uses 1/4 scan with specific pixel mapping */
    uint16_t byteOffset;
    uint8_t bitMask;
    
    /* P10 HUB12 pixel mapping */
    uint16_t row = y % 4;           // Scan row (0-3)
    uint16_t subRow = y / 4;        // Sub-row (0-3)
    uint16_t col = x;
    
    /* Calculate buffer position */
    byteOffset = row * (P10_WIDTH / 8 * 4) + subRow * (P10_WIDTH / 8) + col / 8;
    bitMask = 0x80 >> (col % 8);
    
    if(byteOffset >= P10_BUFFER_SIZE) {
        return;
    }
    
    /* P10 uses inverted logic: 0 = LED on, 1 = LED off */
    if(value) {
        P10_Buffer[byteOffset] &= ~bitMask;  // LED on
    } else {
        P10_Buffer[byteOffset] |= bitMask;   // LED off
    }
}

uint8_t P10_GetPixel(uint16_t x, uint16_t y)
{
    if(x >= P10_WIDTH || y >= P10_HEIGHT) {
        return 0;
    }
    
    uint16_t row = y % 4;
    uint16_t subRow = y / 4;
    uint16_t col = x;
    
    uint16_t byteOffset = row * (P10_WIDTH / 8 * 4) + subRow * (P10_WIDTH / 8) + col / 8;
    uint8_t bitMask = 0x80 >> (col % 8);
    
    if(byteOffset >= P10_BUFFER_SIZE) {
        return 0;
    }
    
    /* Inverted logic */
    return (P10_Buffer[byteOffset] & bitMask) ? 0 : 1;
}

void P10_Refresh(void)
{
    /* Turn off display during update */
    P10_OE_HIGH();
    
    /* Select current row */
    P10_SelectRow(P10_CurrentRow);
    
    /* Calculate start position in buffer for current row */
    uint16_t startOffset = P10_CurrentRow * (P10_WIDTH / 8 * 4);
    
    /* Send data for all 4 sub-rows of current scan row */
    for(uint8_t subRow = 0; subRow < 4; subRow++)
    {
        uint16_t offset = startOffset + subRow * (P10_WIDTH / 8);
        
        /* Send bytes for this row (right to left for proper display) */
        for(int8_t panel = P10_PANELS_X - 1; panel >= 0; panel--)
        {
            for(int8_t byte = 3; byte >= 0; byte--)
            {
                uint16_t idx = offset + panel * 4 + byte;
                if(idx < P10_BUFFER_SIZE) {
                    P10_SendByte(P10_Buffer[idx]);
                }
            }
        }
    }
    
    /* Latch data */
    P10_LAT_HIGH();
    P10_DelayUs(1);
    P10_LAT_LOW();
    
    /* Turn on display */
    P10_OE_LOW();
    
    /* Move to next row */
    P10_CurrentRow = (P10_CurrentRow + 1) % P10_SCAN_RATE;
}

void P10_DrawChar(uint16_t x, uint16_t y, char c)
{
    if(c < 32 || c > 127) {
        c = 32; // Default to space for unsupported characters
    }
    
    const uint8_t* charData = Font5x7[c - 32];
    
    for(uint8_t col = 0; col < 5; col++)
    {
        uint8_t line = charData[col];
        
        for(uint8_t row = 0; row < 7; row++)
        {
            if(line & (1 << row)) {
                P10_SetPixel(x + col, y + row, 1);
            } else {
                P10_SetPixel(x + col, y + row, 0);
            }
        }
    }
}

void P10_DrawString(uint16_t x, uint16_t y, const char* str)
{
    while(*str)
    {
        P10_DrawChar(x, y, *str);
        x += 6; // 5 pixels + 1 space
        str++;
        
        if(x >= P10_WIDTH) {
            break;
        }
    }
}

void P10_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    int16_t dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int16_t dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;
    
    while(1)
    {
        P10_SetPixel(x0, y0, 1);
        
        if(x0 == x1 && y0 == y1) {
            break;
        }
        
        int16_t e2 = 2 * err;
        
        if(e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        
        if(e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void P10_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t fill)
{
    if(fill)
    {
        for(uint16_t j = y; j < y + h; j++)
        {
            for(uint16_t i = x; i < x + w; i++)
            {
                P10_SetPixel(i, j, 1);
            }
        }
    }
    else
    {
        /* Top and bottom lines */
        for(uint16_t i = x; i < x + w; i++)
        {
            P10_SetPixel(i, y, 1);
            P10_SetPixel(i, y + h - 1, 1);
        }
        
        /* Left and right lines */
        for(uint16_t j = y; j < y + h; j++)
        {
            P10_SetPixel(x, j, 1);
            P10_SetPixel(x + w - 1, j, 1);
        }
    }
}

void P10_ScrollText(uint16_t y, const char* str, uint16_t delay_ms)
{
    uint16_t textLen = 0;
    const char* temp = str;
    while(*temp++) textLen++;
    
    uint16_t textWidth = textLen * 6;
    int16_t x = P10_WIDTH;
    
    while(x > -(int16_t)textWidth)
    {
        P10_Clear();
        P10_DrawString(x, y, str);
        
        /* Simple delay */
        for(volatile uint32_t i = 0; i < delay_ms * 1000; i++);
        
        x--;
    }
}

void P10_SetBrightness(uint8_t brightness)
{
    if(brightness > 100) {
        brightness = 100;
    }
    P10_Brightness = brightness;
}

void P10_SetupTimer(void)
{
    /* Enable TIM3 clock */
    P10_RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    
    /* Configure TIM3 for 1ms interrupt (assuming 72MHz clock) */
    P10_TIM3->PSC = 7200 - 1;   // Prescaler: 72MHz / 7200 = 10kHz
    P10_TIM3->ARR = 10 - 1;     // Auto-reload: 10kHz / 10 = 1kHz (1ms)
    
    /* Enable update interrupt */
    P10_TIM3->DIER |= TIM_DIER_UIE;
    
    /* Enable TIM3 interrupt in NVIC */
    P10_NVIC_EnableIRQ(TIM3_IRQn);
    P10_NVIC_SetPriority(TIM3_IRQn, 2);
    
    /* Start timer */
    P10_TIM3->CR1 |= TIM_CR1_CEN;
}

void P10_TimerHandler(void)
{
    /* Clear update interrupt flag */
    P10_TIM3->SR &= ~TIM_SR_UIF;
    
    /* Refresh display */
    P10_Refresh();
}

/* TIM3 Interrupt Handler (can be placed here or in separate file) */
void TIM3_IRQHandler(void)
{
    P10_TimerHandler();
}
