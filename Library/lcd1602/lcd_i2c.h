/**
 ******************************************************************************
 * @file           : lcd_i2c.h
 * @brief          : LCD 1602 I2C Driver for STM32F103C8T6
 ******************************************************************************
 */

#ifndef __LCD_I2C_H
#define __LCD_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ============================================================================
 * LCD I2C Configuration
 * ============================================================================ */

/* I2C Address - Change if your module uses different address */
#define LCD_I2C_ADDR            0x27    // PCF8574 default (try 0x3F for PCF8574A)

/* I2C Pins (I2C1) */
#define LCD_I2C                 I2C1
#define LCD_I2C_SCL_PORT        GPIOB
#define LCD_I2C_SCL_PIN         GPIO_PIN_6      // PB6 - I2C1_SCL
#define LCD_I2C_SDA_PORT        GPIOB
#define LCD_I2C_SDA_PIN         GPIO_PIN_7      // PB7 - I2C1_SDA

/* LCD Dimensions */
#define LCD_COLS                16
#define LCD_ROWS                2

/* ============================================================================
 * LCD Commands
 * ============================================================================ */

/* Commands */
#define LCD_CMD_CLEAR           0x01
#define LCD_CMD_HOME            0x02
#define LCD_CMD_ENTRY_MODE      0x04
#define LCD_CMD_DISPLAY_CTRL    0x08
#define LCD_CMD_CURSOR_SHIFT    0x10
#define LCD_CMD_FUNCTION_SET    0x20
#define LCD_CMD_SET_CGRAM       0x40
#define LCD_CMD_SET_DDRAM       0x80

/* Entry Mode flags */
#define LCD_ENTRY_RIGHT         0x00
#define LCD_ENTRY_LEFT          0x02
#define LCD_ENTRY_SHIFT_INC     0x01
#define LCD_ENTRY_SHIFT_DEC     0x00

/* Display Control flags */
#define LCD_DISPLAY_ON          0x04
#define LCD_DISPLAY_OFF         0x00
#define LCD_CURSOR_ON           0x02
#define LCD_CURSOR_OFF          0x00
#define LCD_BLINK_ON            0x01
#define LCD_BLINK_OFF           0x00

/* Function Set flags */
#define LCD_8BIT_MODE           0x10
#define LCD_4BIT_MODE           0x00
#define LCD_2LINE               0x08
#define LCD_1LINE               0x00
#define LCD_5x10_DOTS           0x04
#define LCD_5x8_DOTS            0x00

/* Backlight */
#define LCD_BACKLIGHT           0x08
#define LCD_NOBACKLIGHT         0x00

/* Enable bit */
#define LCD_EN                  0x04
/* Read/Write bit */
#define LCD_RW                  0x02
/* Register Select bit */
#define LCD_RS                  0x01

/* ============================================================================
 * I2C Register Definitions
 * ============================================================================ */

/* I2C Base Address */
#define I2C1_BASE               ((uint32_t)0x40005400)

/* I2C Register Structure */
typedef struct {
    volatile uint32_t CR1;      // Control register 1
    volatile uint32_t CR2;      // Control register 2
    volatile uint32_t OAR1;     // Own address register 1
    volatile uint32_t OAR2;     // Own address register 2
    volatile uint32_t DR;       // Data register
    volatile uint32_t SR1;      // Status register 1
    volatile uint32_t SR2;      // Status register 2
    volatile uint32_t CCR;      // Clock control register
    volatile uint32_t TRISE;    // Rise time register
} I2C_TypeDef;

#define I2C1                    ((I2C_TypeDef*)I2C1_BASE)

/* I2C CR1 Bits */
#define I2C_CR1_PE              (1 << 0)    // Peripheral enable
#define I2C_CR1_START           (1 << 8)    // Start generation
#define I2C_CR1_STOP            (1 << 9)    // Stop generation
#define I2C_CR1_ACK             (1 << 10)   // Acknowledge enable
#define I2C_CR1_SWRST           (1 << 15)   // Software reset

/* I2C CR2 Bits */
#define I2C_CR2_FREQ_MASK       0x3F        // Peripheral clock frequency

/* I2C SR1 Bits */
#define I2C_SR1_SB              (1 << 0)    // Start bit
#define I2C_SR1_ADDR            (1 << 1)    // Address sent
#define I2C_SR1_BTF             (1 << 2)    // Byte transfer finished
#define I2C_SR1_TXE             (1 << 7)    // Data register empty
#define I2C_SR1_RXNE            (1 << 6)    // Data register not empty

/* I2C CCR Bits */
#define I2C_CCR_FS              (1 << 15)   // Fast mode selection

/* ============================================================================
 * Public Functions
 * ============================================================================ */

/**
 * @brief  Initialize LCD I2C
 * @retval None
 */
void LCD_Init(void);

/**
 * @brief  Clear LCD display
 * @retval None
 */
void LCD_Clear(void);

/**
 * @brief  Set cursor to home position
 * @retval None
 */
void LCD_Home(void);

/**
 * @brief  Set cursor position
 * @param  col: Column (0 to LCD_COLS-1)
 * @param  row: Row (0 to LCD_ROWS-1)
 * @retval None
 */
void LCD_SetCursor(uint8_t col, uint8_t row);

/**
 * @brief  Print a string to LCD
 * @param  str: Null-terminated string
 * @retval None
 */
void LCD_Print(const char* str);

/**
 * @brief  Print a single character
 * @param  c: Character to print
 * @retval None
 */
void LCD_PrintChar(char c);

/**
 * @brief  Print an integer number
 * @param  num: Number to print
 * @retval None
 */
void LCD_PrintInt(int32_t num);

/**
 * @brief  Turn on/off display
 * @param  on: 1 = on, 0 = off
 * @retval None
 */
void LCD_Display(uint8_t on);

/**
 * @brief  Turn on/off cursor
 * @param  on: 1 = on, 0 = off
 * @retval None
 */
void LCD_Cursor(uint8_t on);

/**
 * @brief  Turn on/off cursor blink
 * @param  on: 1 = on, 0 = off
 * @retval None
 */
void LCD_Blink(uint8_t on);

/**
 * @brief  Turn on/off backlight
 * @param  on: 1 = on, 0 = off
 * @retval None
 */
void LCD_Backlight(uint8_t on);

/**
 * @brief  Create custom character
 * @param  location: Character location (0-7)
 * @param  charmap: Array of 8 bytes defining the character
 * @retval None
 */
void LCD_CreateChar(uint8_t location, uint8_t charmap[]);

/**
 * @brief  Scroll display left
 * @retval None
 */
void LCD_ScrollLeft(void);

/**
 * @brief  Scroll display right
 * @retval None
 */
void LCD_ScrollRight(void);

#ifdef __cplusplus
}
#endif

#endif /* __LCD_I2C_H */
