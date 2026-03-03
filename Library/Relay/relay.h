/**
 ******************************************************************************
 * @file           : relay.h
 * @brief          : 4-Channel Relay Control Driver Header
 *                   Compatible with STM32F103C8T6
 ******************************************************************************
 * @attention
 *
 * 4 Relay outputs on GPIOA:
 *   Relay 0: PA9   — Cổng relay 1 (cửa 1 / thiết bị 1)
 *   Relay 1: PA10  — Cổng relay 2 (cửa 2 / thiết bị 2)
 *   Relay 2: PA11  — Cổng relay 3 (cửa 3 / thiết bị 3)
 *   Relay 3: PA12  — Cổng relay 4 (cửa 4 / thiết bị 4)
 *
 * Output: Push-Pull, Active LOW (most relay modules are active-low)
 *         Set RELAY_ACTIVE_HIGH to 1 if your module is active-high
 *
 * Relay modules typically need 5V VCC but 3.3V signal from STM32
 * works with most optocoupler-isolated relay boards.
 *
 ******************************************************************************
 */

#ifndef __RELAY_H
#define __RELAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ============================================================================
 * STM32F103 Peripheral Definitions
 * ============================================================================ */

#ifndef PERIPH_BASE
#define PERIPH_BASE                 ((uint32_t)0x40000000)
#define APB2PERIPH_BASE             (PERIPH_BASE + 0x10000)
#define AHBPERIPH_BASE              (PERIPH_BASE + 0x20000)
#endif

#ifndef GPIOA_BASE
#define GPIOA_BASE                  (APB2PERIPH_BASE + 0x0800)
#endif

#ifndef RCC_BASE
#define RCC_BASE                    (AHBPERIPH_BASE + 0x1000)
#endif

/* ============================================================================
 * GPIO / RCC Register Structures
 * ============================================================================ */

#ifndef __RELAY_GPIO_TYPEDEF__
#define __RELAY_GPIO_TYPEDEF__
typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} RELAY_GPIO_TypeDef;
#endif

#ifndef __RELAY_RCC_TYPEDEF__
#define __RELAY_RCC_TYPEDEF__
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
} RELAY_RCC_TypeDef;
#endif

/* ============================================================================
 * Configuration
 * ============================================================================ */

#define RELAY_NUM_CHANNELS          4       // Number of relay channels

/**
 * Relay active level:
 *   0 = Active LOW  (most common — relay module with optocoupler)
 *   1 = Active HIGH (some relay boards without optocoupler)
 */
#define RELAY_ACTIVE_HIGH           0

/* ============================================================================
 * Pin Definitions — All on GPIOA
 * ============================================================================ */

#define RELAY0_PIN                  9       // PA9  — Relay 1
#define RELAY1_PIN                  10      // PA10 — Relay 2
#define RELAY2_PIN                  11      // PA11 — Relay 3
#define RELAY3_PIN                  12      // PA12 — Relay 4

#define RELAY_GPIO                  ((RELAY_GPIO_TypeDef *)GPIOA_BASE)
#define RELAY_RCC                   ((RELAY_RCC_TypeDef *)RCC_BASE)

/* ============================================================================
 * Relay State
 * ============================================================================ */

#define RELAY_OFF                   0
#define RELAY_ON                    1

/* ============================================================================
 * Data Structures
 * ============================================================================ */

/**
 * @brief  Relay channel status
 */
typedef struct {
    uint8_t  state;             // RELAY_ON or RELAY_OFF
    uint32_t onTime;            // Time (loop counts) relay has been ON (for auto-off)
    uint32_t autoOffDelay;      // Auto-off delay (0 = disabled, >0 = loop counts)
} Relay_Channel_t;

/**
 * @brief  Relay system status
 */
typedef struct {
    Relay_Channel_t ch[RELAY_NUM_CHANNELS];
    uint8_t  initialized;       // 1 if RELAY_Init() was called
} Relay_System_t;

/* ============================================================================
 * Public Functions
 * ============================================================================ */

/**
 * @brief  Initialize all 4 relay GPIO pins as output
 *         All relays start in OFF state
 */
void RELAY_Init(void);

/**
 * @brief  Turn ON a relay
 * @param  ch: Channel number (0-3)
 */
void RELAY_On(uint8_t ch);

/**
 * @brief  Turn OFF a relay
 * @param  ch: Channel number (0-3)
 */
void RELAY_Off(uint8_t ch);

/**
 * @brief  Toggle a relay (ON→OFF or OFF→ON)
 * @param  ch: Channel number (0-3)
 */
void RELAY_Toggle(uint8_t ch);

/**
 * @brief  Set relay state
 * @param  ch: Channel number (0-3)
 * @param  state: RELAY_ON or RELAY_OFF
 */
void RELAY_Set(uint8_t ch, uint8_t state);

/**
 * @brief  Get relay state
 * @param  ch: Channel number (0-3)
 * @retval RELAY_ON or RELAY_OFF
 */
uint8_t RELAY_Get(uint8_t ch);

/**
 * @brief  Turn ON all relays
 */
void RELAY_AllOn(void);

/**
 * @brief  Turn OFF all relays
 */
void RELAY_AllOff(void);

/**
 * @brief  Set all relays at once using bitmask
 * @param  mask: Bit0=Relay0, Bit1=Relay1, etc. (1=ON, 0=OFF)
 */
void RELAY_SetMask(uint8_t mask);

/**
 * @brief  Get all relay states as bitmask
 * @retval Bitmask: Bit0=Relay0, Bit1=Relay1, etc.
 */
uint8_t RELAY_GetMask(void);

/**
 * @brief  Turn ON a relay with auto-off after specified delay
 * @param  ch: Channel number (0-3)
 * @param  delay: Auto-off delay in loop counts (call RELAY_Process() each loop)
 *                Typical: 720000 ≈ 1 second at 72MHz
 */
void RELAY_Pulse(uint8_t ch, uint32_t delay);

/**
 * @brief  Process auto-off timers — call in main loop
 *         Decrements counters and turns off relays when expired
 */
void RELAY_Process(void);

/**
 * @brief  Test all relays sequentially (ON-delay-OFF for each)
 * @param  delay_ms_val: Delay in approximate milliseconds for each relay
 */
void RELAY_TestAll(uint32_t delay_ms_val);

/**
 * @brief  Check if relay system is initialized
 * @retval 1 = initialized, 0 = not initialized
 */
uint8_t RELAY_IsInitialized(void);

/**
 * @brief  Format relay status as string "R:1010" (1=ON, 0=OFF)
 * @param  buf: Output buffer (min 7 bytes: "R:" + 4 digits + null)
 */
void RELAY_FormatStatus(char *buf);

#ifdef __cplusplus
}
#endif

#endif /* __RELAY_H */
