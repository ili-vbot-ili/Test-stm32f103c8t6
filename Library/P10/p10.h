/**
 ******************************************************************************
 * @file           : p10.h
 * @brief          : P10 LED Matrix Module Header (HUB12 Interface)
 *                   Compatible with STM32F103C8T6
 ******************************************************************************
 * @attention
 *
 * P10 LED Panel Connection:
 * - Panel size: 32x16 pixels (1/4 scan)
 * - Interface: HUB12
 *
 * Pin Configuration (Default - can be modified):
 * - OE (Output Enable)  : PB10 (Active LOW)
 * - A (Row Select)      : PB11
 * - B (Row Select)      : PB12
 * - CLK (Clock)         : PB13 (SPI2_SCK)
 * - DATA (MOSI)         : PB15 (SPI2_MOSI)
 * - LAT/STB (Latch)     : PB14
 *
 * Note: Uses GPIOB to avoid conflict with W5500 which uses GPIOA (SPI1)
 *
 ******************************************************************************
 */

#ifndef __P10_H
#define __P10_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ============================================================================
 * STM32F103 Peripheral Definitions (if not already defined)
 * ============================================================================ */

#ifndef PERIPH_BASE
#define PERIPH_BASE                 ((uint32_t)0x40000000)
#define APB1PERIPH_BASE             PERIPH_BASE
#define APB2PERIPH_BASE             (PERIPH_BASE + 0x10000)
#define AHBPERIPH_BASE              (PERIPH_BASE + 0x20000)
#endif

#ifndef GPIOB_BASE
#define GPIOB_BASE                  (APB2PERIPH_BASE + 0x0C00)
#endif

#ifndef TIM2_BASE
#define TIM2_BASE                   (APB1PERIPH_BASE + 0x0000)
#define TIM3_BASE                   (APB1PERIPH_BASE + 0x0400)
#endif

#ifndef RCC_BASE
#define RCC_BASE                    (AHBPERIPH_BASE + 0x1000)
#endif

/* ============================================================================
 * GPIO Register Structure (if not already defined)
 * ============================================================================ */

#ifndef __GPIO_TYPEDEF__
#define __GPIO_TYPEDEF__
typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} P10_GPIO_TypeDef;
#endif

/* ============================================================================
 * Timer Register Structure
 * ============================================================================ */

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
    volatile uint32_t RCR;
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
    volatile uint32_t BDTR;
    volatile uint32_t DCR;
    volatile uint32_t DMAR;
} P10_TIM_TypeDef;

/* ============================================================================
 * RCC Register Structure (if not already defined)
 * ============================================================================ */

#ifndef __RCC_TYPEDEF__
#define __RCC_TYPEDEF__
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t APB1RSTR;
    volatile uint32_t AHBENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB1ENR;
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
} P10_RCC_TypeDef;
#endif

/* ============================================================================
 * NVIC Register Structure
 * ============================================================================ */

#define NVIC_BASE                   ((uint32_t)0xE000E100)

typedef struct {
    volatile uint32_t ISER[8];      // Interrupt Set Enable Registers
    uint32_t RESERVED0[24];
    volatile uint32_t ICER[8];      // Interrupt Clear Enable Registers
    uint32_t RESERVED1[24];
    volatile uint32_t ISPR[8];      // Interrupt Set Pending Registers
    uint32_t RESERVED2[24];
    volatile uint32_t ICPR[8];      // Interrupt Clear Pending Registers
    uint32_t RESERVED3[24];
    volatile uint32_t IABR[8];      // Interrupt Active Bit Registers
    uint32_t RESERVED4[56];
    volatile uint8_t  IP[240];      // Interrupt Priority Registers
} P10_NVIC_TypeDef;

/* ============================================================================
 * Peripheral Instances for P10
 * ============================================================================ */

#define P10_GPIOB                   ((P10_GPIO_TypeDef *)GPIOB_BASE)
#define P10_TIM3                    ((P10_TIM_TypeDef *)TIM3_BASE)
#define P10_RCC                     ((P10_RCC_TypeDef *)RCC_BASE)
#define P10_NVIC                    ((P10_NVIC_TypeDef *)NVIC_BASE)

/* ============================================================================
 * RCC Clock Enable Bits
 * ============================================================================ */

#ifndef RCC_APB2ENR_IOPBEN
#define RCC_APB2ENR_IOPBEN          ((uint32_t)0x00000008)
#endif

#ifndef RCC_APB1ENR_TIM3EN
#define RCC_APB1ENR_TIM3EN          ((uint32_t)0x00000002)
#endif

/* Timer bits */
#define TIM_CR1_CEN                 ((uint16_t)0x0001)
#define TIM_DIER_UIE                ((uint16_t)0x0001)
#define TIM_SR_UIF                  ((uint16_t)0x0001)

/* TIM3 IRQ number */
#define TIM3_IRQn                   29

/* ============================================================================
 * P10 Configuration
 * ============================================================================ */

/* Number of panels (horizontal x vertical) */
#define P10_PANELS_X        1       /* Number of panels in X direction */
#define P10_PANELS_Y        1       /* Number of panels in Y direction */

/* Panel dimensions */
#define P10_PANEL_WIDTH     32      /* Single panel width in pixels */
#define P10_PANEL_HEIGHT    16      /* Single panel height in pixels */

/* Total display dimensions */
#define P10_WIDTH           (P10_PANEL_WIDTH * P10_PANELS_X)
#define P10_HEIGHT          (P10_PANEL_HEIGHT * P10_PANELS_Y)

/* Buffer size for frame buffer */
#define P10_BUFFER_SIZE     ((P10_WIDTH * P10_HEIGHT) / 8)

/* Scan rate (1/4 scan for P10) */
#define P10_SCAN_RATE       4

/* ============================================================================
 * GPIO Pin Definitions (Using GPIOB to avoid conflict with W5500 on GPIOA)
 * ============================================================================ */

/* Using GPIOB for P10 control */
#define P10_GPIO_PORT       P10_GPIOB

/* Pin definitions - Using PB10-PB15 */
#define P10_PIN_OE          (1U << 10)  /* PB10 - Output Enable (Active LOW) */
#define P10_PIN_A           (1U << 11)  /* PB11 - Row Select A */
#define P10_PIN_B           (1U << 12)  /* PB12 - Row Select B */
#define P10_PIN_CLK         (1U << 13)  /* PB13 - Clock (SPI2_SCK) */
#define P10_PIN_LAT         (1U << 14)  /* PB14 - Latch */
#define P10_PIN_DATA        (1U << 15)  /* PB15 - Data (SPI2_MOSI) */

/* ============================================================================
 * P10 Macros
 * ============================================================================ */

#define P10_OE_HIGH()       (P10_GPIO_PORT->BSRR = P10_PIN_OE)
#define P10_OE_LOW()        (P10_GPIO_PORT->BRR = P10_PIN_OE)

#define P10_A_HIGH()        (P10_GPIO_PORT->BSRR = P10_PIN_A)
#define P10_A_LOW()         (P10_GPIO_PORT->BRR = P10_PIN_A)

#define P10_B_HIGH()        (P10_GPIO_PORT->BSRR = P10_PIN_B)
#define P10_B_LOW()         (P10_GPIO_PORT->BRR = P10_PIN_B)

#define P10_LAT_HIGH()      (P10_GPIO_PORT->BSRR = P10_PIN_LAT)
#define P10_LAT_LOW()       (P10_GPIO_PORT->BRR = P10_PIN_LAT)

#define P10_CLK_HIGH()      (P10_GPIO_PORT->BSRR = P10_PIN_CLK)
#define P10_CLK_LOW()       (P10_GPIO_PORT->BRR = P10_PIN_CLK)

#define P10_DATA_HIGH()     (P10_GPIO_PORT->BSRR = P10_PIN_DATA)
#define P10_DATA_LOW()      (P10_GPIO_PORT->BRR = P10_PIN_DATA)

/* ============================================================================
 * P10 Function Prototypes
 * ============================================================================ */

/**
 * @brief  Initialize P10 LED Matrix
 */
void P10_Init(void);

/**
 * @brief  Clear display (all LEDs off)
 */
void P10_Clear(void);

/**
 * @brief  Fill display (all LEDs on)
 */
void P10_Fill(void);

/**
 * @brief  Set a single pixel
 * @param  x: X coordinate (0 to P10_WIDTH-1)
 * @param  y: Y coordinate (0 to P10_HEIGHT-1)
 * @param  value: 1 = ON, 0 = OFF
 */
void P10_SetPixel(uint16_t x, uint16_t y, uint8_t value);

/**
 * @brief  Get pixel value
 * @param  x: X coordinate
 * @param  y: Y coordinate
 * @retval Pixel value (1 or 0)
 */
uint8_t P10_GetPixel(uint16_t x, uint16_t y);

/**
 * @brief  Refresh display - MUST be called periodically (e.g., in Timer interrupt)
 *         Recommended: Call every 1-2ms for good brightness
 */
void P10_Refresh(void);

/**
 * @brief  Draw a character at position
 * @param  x: X coordinate
 * @param  y: Y coordinate
 * @param  c: Character to draw
 */
void P10_DrawChar(uint16_t x, uint16_t y, char c);

/**
 * @brief  Draw a string at position
 * @param  x: X coordinate
 * @param  y: Y coordinate
 * @param  str: String to draw
 */
void P10_DrawString(uint16_t x, uint16_t y, const char* str);

/**
 * @brief  Draw a line
 * @param  x0: Start X
 * @param  y0: Start Y
 * @param  x1: End X
 * @param  y1: End Y
 */
void P10_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/**
 * @brief  Draw a rectangle
 * @param  x: Top-left X
 * @param  y: Top-left Y
 * @param  w: Width
 * @param  h: Height
 * @param  fill: 1 = filled, 0 = outline only
 */
void P10_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t fill);

/**
 * @brief  Scroll text horizontally
 * @param  y: Y position
 * @param  str: Text to scroll
 * @param  delay_ms: Delay between scroll steps
 */
void P10_ScrollText(uint16_t y, const char* str, uint16_t delay_ms);

/**
 * @brief  Set display brightness (using PWM on OE pin)
 * @param  brightness: 0-100 (percent)
 */
void P10_SetBrightness(uint8_t brightness);

/**
 * @brief  Timer interrupt handler for P10 refresh
 *         Call this from TIM2_IRQHandler or similar
 */
void P10_TimerHandler(void);

/**
 * @brief  Setup Timer for automatic refresh
 */
void P10_SetupTimer(void);

/**
 * @brief  NVIC helper functions
 */
static inline void P10_NVIC_EnableIRQ(uint8_t IRQn) {
    P10_NVIC->ISER[IRQn >> 5] = (1UL << (IRQn & 0x1F));
}

static inline void P10_NVIC_SetPriority(uint8_t IRQn, uint8_t priority) {
    P10_NVIC->IP[IRQn] = (priority << 4) & 0xFF;
}

#ifdef __cplusplus
}
#endif

#endif /* __P10_H */
