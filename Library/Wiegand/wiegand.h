/**
 ******************************************************************************
 * @file           : wiegand.h
 * @brief          : Multi-Port Wiegand Protocol Reader Driver Header
 *                   Compatible with STM32F103C8T6
 ******************************************************************************
 * @attention
 *
 * Wiegand Interface for RFID Card Readers (2 ports)
 * - Supports: Wiegand 26-bit, 34-bit, 37-bit
 * - Interface: 2-wire (D0, D1) per port, polling-based
 * - Idle state: both lines HIGH
 * - Data pulse: LOW for ~50us, interval ~2ms between bits
 * - Timeout: 25ms after last bit = frame complete
 *
 * Pin Configuration (all GPIOA):
 *   Port 0: PA3 (D0), PA8  (D1) — Cổng chính
 *   Port 1: PA0 (D0), PA1  (D1) — Cổng phụ 1
 *
 * JTAG giữ nguyên (PA13/PA14/PA15/PB3/PB4)
 *
 * Wiegand 26-bit Format (H10301):
 * ┌──┬──────────┬──────────────────┬──┐
 * │EP│ Facility │    Card Number   │OP│
 * │1 │  8 bits  │     16 bits      │1 │
 * └──┴──────────┴──────────────────┴──┘
 *
 ******************************************************************************
 */

#ifndef __WIEGAND_H
#define __WIEGAND_H

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

#ifndef AFIO_BASE
#define AFIO_BASE                   (APB2PERIPH_BASE + 0x0000)
#endif

/* ============================================================================
 * GPIO / RCC / AFIO Register Structures
 * ============================================================================ */

#ifndef __WG_GPIO_TYPEDEF__
#define __WG_GPIO_TYPEDEF__
typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} WG_GPIO_TypeDef;
#endif

#ifndef __WG_RCC_TYPEDEF__
#define __WG_RCC_TYPEDEF__
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
} WG_RCC_TypeDef;
#endif

#ifndef __WG_AFIO_TYPEDEF__
#define __WG_AFIO_TYPEDEF__
typedef struct {
    volatile uint32_t EVCR;
    volatile uint32_t MAPR;
    volatile uint32_t EXTICR[4];
    volatile uint32_t RESERVED;
    volatile uint32_t MAPR2;
} WG_AFIO_TypeDef;
#endif

/* ============================================================================
 * Configuration
 * ============================================================================ */

#define WG_NUM_PORTS                2       // Number of Wiegand reader ports
#define WG_MAX_BITS                 64      // Max bits per frame
#define WG_TIMEOUT_LOOPS            180000  // ~25ms timeout at 72MHz

/* ============================================================================
 * Pin Definitions — All on GPIOA
 * ============================================================================ */

/* Port 0: PA3 (D0), PA8 (D1) */
#define WG0_D0_PIN                  3
#define WG0_D1_PIN                  8

/* Port 1: PA0 (D0), PA1 (D1) */
#define WG1_D0_PIN                  0
#define WG1_D1_PIN                  1

#define WG_GPIO                     ((WG_GPIO_TypeDef *)GPIOA_BASE)
#define WG_RCC                      ((WG_RCC_TypeDef *)RCC_BASE)
#define WG_AFIO                     ((WG_AFIO_TypeDef *)AFIO_BASE)

/* ============================================================================
 * Wiegand Format Types
 * ============================================================================ */

#define WG_FORMAT_UNKNOWN           0
#define WG_FORMAT_26BIT             26
#define WG_FORMAT_34BIT             34
#define WG_FORMAT_37BIT             37

/* ============================================================================
 * Data Structures
 * ============================================================================ */

/**
 * @brief  Wiegand decoded card data
 */
typedef struct {
    uint32_t cardNumber;        // Card number
    uint16_t facilityCode;      // Facility / Site code
    uint32_t rawData;           // Raw data (without parity bits)
    uint64_t rawBits;           // All raw bits received
    uint8_t  bitCount;          // Number of bits received
    uint8_t  format;            // WG_FORMAT_26BIT, 34BIT, etc.
    uint8_t  valid;             // 1 = parity check passed
    uint8_t  port;              // Which port (0-1) this card was read from
} WG_CardData_t;

/**
 * @brief  Wiegand reader state (per port)
 */
typedef enum {
    WG_STATE_IDLE = 0,
    WG_STATE_RECEIVING,
    WG_STATE_COMPLETE
} WG_State_t;

/* ============================================================================
 * Public Functions
 * ============================================================================ */

/**
 * @brief  Initialize all Wiegand reader ports (GPIO config)
 *         JTAG is preserved (PA15/PB3/PB4 not touched)
 */
void WG_Init(void);

/**
 * @brief  Initialize a specific Wiegand port only
 * @param  port: Port number (0-1)
 */
void WG_InitPort(uint8_t port);

/**
 * @brief  Process all Wiegand ports — call frequently in main loop
 * @retval Bitmask of ports with new data (bit0=port0, bit1=port1)
 *         0 = no new data on any port
 */
uint8_t WG_Process(void);

/**
 * @brief  Process a specific Wiegand port
 * @param  port: Port number (0-1)
 * @retval 1 = new card data available on this port, 0 = still waiting
 */
uint8_t WG_ProcessPort(uint8_t port);

/**
 * @brief  Get the last decoded card data from a specific port
 * @param  port: Port number (0-1)
 * @param  card: Pointer to card data structure (output)
 * @retval 1 = valid data copied, 0 = no data available
 */
uint8_t WG_GetCard(uint8_t port, WG_CardData_t *card);

/**
 * @brief  Check if a card has been read on a specific port
 * @param  port: Port number (0-1)
 * @retval 1 = card data available, 0 = no data
 */
uint8_t WG_Available(uint8_t port);

/**
 * @brief  Check if any port has card data available
 * @retval Port number (0-1) with data, or 0xFF if none
 */
uint8_t WG_AnyAvailable(void);

/**
 * @brief  Reset a specific port state
 * @param  port: Port number (0-1)
 */
void WG_Reset(uint8_t port);

/**
 * @brief  Reset all ports
 */
void WG_ResetAll(void);

/**
 * @brief  Get the number of bits received on a port
 * @param  port: Port number (0-1)
 * @retval Number of bits (0-64)
 */
uint8_t WG_GetBitCount(uint8_t port);

/**
 * @brief  Get raw bits from a port
 * @param  port: Port number (0-1)
 * @retval Raw bits received
 */
uint64_t WG_GetRawBits(uint8_t port);

/**
 * @brief  Decode raw bits into card data
 */
uint8_t WG_Decode(uint64_t rawBits, uint8_t bitCount, WG_CardData_t *card);

/**
 * @brief  Verify parity for Wiegand 26-bit format
 */
uint8_t WG_CheckParity26(uint32_t rawBits);

/**
 * @brief  Verify parity for Wiegand 34-bit format
 */
uint8_t WG_CheckParity34(uint64_t rawBits);

/**
 * @brief  Format card data as string "P0 FC:xxx ID:xxxxx"
 * @param  card: Card data source
 * @param  buf: Output buffer (min 28 bytes)
 */
void WG_FormatCard(const WG_CardData_t *card, char *buf);

/**
 * @brief  Format card number only as decimal string
 */
void WG_FormatCardNumber(const WG_CardData_t *card, char *buf);

#ifdef __cplusplus
}
#endif

#endif /* __WIEGAND_H */
