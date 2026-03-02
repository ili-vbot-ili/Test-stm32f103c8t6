/**
 ******************************************************************************
 * @file           : lcd_i2c.c
 * @brief          : LCD 1602 I2C Driver for STM32F103C8T6
 ******************************************************************************
 */

#include "lcd_i2c.h"
#include "w5500_conf.h"  // For GPIO definitions and RCC

/* ============================================================================
 * Private Variables
 * ============================================================================ */

static uint8_t _displayControl = LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF;
static uint8_t _displayMode = LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_DEC;
static uint8_t _backlightVal = LCD_BACKLIGHT;
static uint8_t _lcd_connected = 0;  // Auto-detect flag

#define I2C_TIMEOUT  5000

/* ============================================================================
 * Private Functions - Delay
 * ============================================================================ */

static void LCD_DelayUs(uint32_t us)
{
    /* Simple delay - adjust for your clock speed (72MHz assumed) */
    us *= 12;  // Approximate cycles for 72MHz
    while(us--) {
        __asm volatile("nop");
    }
}

static void LCD_DelayMs(uint32_t ms)
{
    while(ms--) {
        LCD_DelayUs(1000);
    }
}

/* ============================================================================
 * Private Functions - I2C
 * ============================================================================ */

/**
 * @brief  Initialize I2C1 peripheral
 */
static void LCD_I2C_Init(void)
{
    /* Enable GPIOB and I2C1 clocks */
    RCC->APB2ENR |= (1 << 3);   // GPIOB clock enable
    RCC->APB1ENR |= (1 << 21);  // I2C1 clock enable
    
    /* Configure PB6 (SCL) and PB7 (SDA) as Alternate Function Open-Drain */
    /* CNF = 11 (AF Open-Drain), MODE = 11 (50MHz output) */
    GPIOB->CRL &= ~(0xFF << 24);    // Clear PB6, PB7 config
    GPIOB->CRL |= (0xFF << 24);     // Set AF Open-Drain, 50MHz
    
    /* Reset I2C */
    I2C1->CR1 |= I2C_CR1_SWRST;
    I2C1->CR1 &= ~I2C_CR1_SWRST;
    
    /* Configure I2C clock - assuming 36MHz APB1 clock */
    I2C1->CR2 = 36;                 // APB1 frequency in MHz
    
    /* Configure I2C speed (100kHz Standard mode) */
    /* CCR = APB1_freq / (2 * I2C_freq) = 36MHz / (2 * 100kHz) = 180 */
    I2C1->CCR = 180;
    
    /* Configure rise time */
    /* TRISE = (max_rise_time / t_APB1) + 1 = (1000ns / 27.7ns) + 1 = 37 */
    I2C1->TRISE = 37;
    
    /* Enable I2C */
    I2C1->CR1 |= I2C_CR1_PE;
}

/**
 * @brief  Send START condition
 */
static uint8_t LCD_I2C_Start(void)
{
    I2C1->CR1 |= I2C_CR1_ACK;
    I2C1->CR1 |= I2C_CR1_START;
    
    /* Wait for start bit with timeout */
    volatile uint32_t t = I2C_TIMEOUT;
    while(!(I2C1->SR1 & I2C_SR1_SB) && --t);
    return (t > 0) ? 1 : 0;
}

/**
 * @brief  Send STOP condition
 */
static void LCD_I2C_Stop(void)
{
    I2C1->CR1 |= I2C_CR1_STOP;
}

/**
 * @brief  Send address
 */
static uint8_t LCD_I2C_SendAddr(uint8_t addr)
{
    I2C1->DR = addr << 1;  // Write mode (LSB = 0)
    
    /* Wait for address sent with timeout */
    volatile uint32_t t = I2C_TIMEOUT;
    while(!(I2C1->SR1 & (I2C_SR1_ADDR | (1 << 10)/*AF*/)) && --t);
    
    if((t == 0) || (I2C1->SR1 & (1 << 10)/*AF*/)) {
        /* NACK or timeout - device not present */
        I2C1->SR1 &= ~(1 << 10);  // Clear AF flag
        I2C1->CR1 |= I2C_CR1_STOP;
        return 0;
    }
    
    /* Clear ADDR flag by reading SR1 and SR2 */
    (void)I2C1->SR1;
    (void)I2C1->SR2;
    return 1;
}

/**
 * @brief  Send data byte
 */
static void LCD_I2C_SendData(uint8_t data)
{
    /* Wait for TXE with timeout */
    volatile uint32_t t = I2C_TIMEOUT;
    while(!(I2C1->SR1 & I2C_SR1_TXE) && --t);
    if(t == 0) return;
    
    I2C1->DR = data;
    
    /* Wait for BTF with timeout */
    t = I2C_TIMEOUT;
    while(!(I2C1->SR1 & I2C_SR1_BTF) && --t);
}

/**
 * @brief  Write byte to I2C
 */
static void LCD_I2C_Write(uint8_t data)
{
    if(!_lcd_connected) return;
    if(!LCD_I2C_Start()) { _lcd_connected = 0; return; }
    if(!LCD_I2C_SendAddr(LCD_I2C_ADDR)) { _lcd_connected = 0; return; }
    LCD_I2C_SendData(data);
    LCD_I2C_Stop();
}

/* ============================================================================
 * Private Functions - LCD
 * ============================================================================ */

/**
 * @brief  Write 4 bits to LCD
 */
static void LCD_Write4Bits(uint8_t value)
{
    LCD_I2C_Write(value | _backlightVal);
    
    /* Pulse enable */
    LCD_I2C_Write(value | LCD_EN | _backlightVal);
    LCD_DelayUs(1);
    LCD_I2C_Write((value & ~LCD_EN) | _backlightVal);
    LCD_DelayUs(50);
}

/**
 * @brief  Send byte to LCD
 */
static void LCD_Send(uint8_t value, uint8_t mode)
{
    uint8_t highNibble = value & 0xF0;
    uint8_t lowNibble = (value << 4) & 0xF0;
    
    LCD_Write4Bits(highNibble | mode);
    LCD_Write4Bits(lowNibble | mode);
}

/**
 * @brief  Send command to LCD
 */
static void LCD_Command(uint8_t cmd)
{
    LCD_Send(cmd, 0);
}

/**
 * @brief  Send data to LCD
 */
static void LCD_Data(uint8_t data)
{
    LCD_Send(data, LCD_RS);
}

/* ============================================================================
 * Public Functions
 * ============================================================================ */

void LCD_Init(void)
{
    /* Initialize I2C */
    LCD_I2C_Init();
    
    /* Wait for LCD power up */
    LCD_DelayMs(50);
    
    /* Probe I2C device to check if LCD is connected */
    _lcd_connected = 1;  // Temporarily enable for probe
    if(!LCD_I2C_Start()) { _lcd_connected = 0; return; }
    if(!LCD_I2C_SendAddr(LCD_I2C_ADDR)) { _lcd_connected = 0; return; }
    LCD_I2C_Stop();
    /* LCD detected! */
    
    /* Initialize in 4-bit mode */
    LCD_Write4Bits(0x30);
    LCD_DelayMs(5);
    
    LCD_Write4Bits(0x30);
    LCD_DelayMs(5);
    
    LCD_Write4Bits(0x30);
    LCD_DelayUs(150);
    
    LCD_Write4Bits(0x20);  // Set 4-bit mode
    
    /* Function set: 4-bit, 2 lines, 5x8 dots */
    LCD_Command(LCD_CMD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2LINE | LCD_5x8_DOTS);
    
    /* Display control: display on, cursor off, blink off */
    _displayControl = LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF;
    LCD_Command(LCD_CMD_DISPLAY_CTRL | _displayControl);
    
    /* Clear display */
    LCD_Clear();
    
    /* Entry mode: left to right */
    _displayMode = LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_DEC;
    LCD_Command(LCD_CMD_ENTRY_MODE | _displayMode);
    
    LCD_DelayMs(2);
}

void LCD_Clear(void)
{
    if(!_lcd_connected) return;
    LCD_Command(LCD_CMD_CLEAR);
    LCD_DelayMs(2);
}

void LCD_Home(void)
{
    if(!_lcd_connected) return;
    LCD_Command(LCD_CMD_HOME);
    LCD_DelayMs(2);
}

void LCD_SetCursor(uint8_t col, uint8_t row)
{
    if(!_lcd_connected) return;
    uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    
    if(row >= LCD_ROWS) {
        row = LCD_ROWS - 1;
    }
    
    LCD_Command(LCD_CMD_SET_DDRAM | (col + row_offsets[row]));
}

void LCD_Print(const char* str)
{
    if(!_lcd_connected) return;
    while(*str) {
        LCD_Data(*str++);
    }
}

void LCD_PrintChar(char c)
{
    if(!_lcd_connected) return;
    LCD_Data(c);
}

void LCD_PrintInt(int32_t num)
{
    if(!_lcd_connected) return;
    char buffer[12];
    int i = 0;
    int isNegative = 0;
    
    if(num == 0) {
        LCD_Data('0');
        return;
    }
    
    if(num < 0) {
        isNegative = 1;
        num = -num;
    }
    
    while(num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    if(isNegative) {
        LCD_Data('-');
    }
    
    while(i > 0) {
        LCD_Data(buffer[--i]);
    }
}

void LCD_Display(uint8_t on)
{
    if(on) {
        _displayControl |= LCD_DISPLAY_ON;
    } else {
        _displayControl &= ~LCD_DISPLAY_ON;
    }
    LCD_Command(LCD_CMD_DISPLAY_CTRL | _displayControl);
}

void LCD_Cursor(uint8_t on)
{
    if(on) {
        _displayControl |= LCD_CURSOR_ON;
    } else {
        _displayControl &= ~LCD_CURSOR_ON;
    }
    LCD_Command(LCD_CMD_DISPLAY_CTRL | _displayControl);
}

void LCD_Blink(uint8_t on)
{
    if(on) {
        _displayControl |= LCD_BLINK_ON;
    } else {
        _displayControl &= ~LCD_BLINK_ON;
    }
    LCD_Command(LCD_CMD_DISPLAY_CTRL | _displayControl);
}

void LCD_Backlight(uint8_t on)
{
    if(on) {
        _backlightVal = LCD_BACKLIGHT;
    } else {
        _backlightVal = LCD_NOBACKLIGHT;
    }
    LCD_I2C_Write(_backlightVal);
}

void LCD_CreateChar(uint8_t location, uint8_t charmap[])
{
    location &= 0x07;  // Only 8 locations (0-7)
    LCD_Command(LCD_CMD_SET_CGRAM | (location << 3));
    
    for(int i = 0; i < 8; i++) {
        LCD_Data(charmap[i]);
    }
}

void LCD_ScrollLeft(void)
{
    if(!_lcd_connected) return;
    LCD_Command(LCD_CMD_CURSOR_SHIFT | 0x08);
}

void LCD_ScrollRight(void)
{
    if(!_lcd_connected) return;
    LCD_Command(LCD_CMD_CURSOR_SHIFT | 0x0C);
}

uint8_t LCD_IsConnected(void)
{
    return _lcd_connected;
}
