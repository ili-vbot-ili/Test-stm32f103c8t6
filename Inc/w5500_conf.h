/**
 ******************************************************************************
 * @file           : w5500_conf.h
 * @brief          : W5500 Hardware Configuration for STM32F103C8T6
 ******************************************************************************
 */

#ifndef __W5500_CONF_H
#define __W5500_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ============================================================================
 * STM32F103C8T6 GPIO Pin Definitions
 * ============================================================================ */

/* SPI1 Pins for W5500 */
#define W5500_SPI                   SPI1

#define W5500_SCK_PORT              GPIOA
#define W5500_SCK_PIN               GPIO_PIN_5      // PA5 - SPI1_SCK

#define W5500_MISO_PORT             GPIOA
#define W5500_MISO_PIN              GPIO_PIN_6      // PA6 - SPI1_MISO

#define W5500_MOSI_PORT             GPIOA
#define W5500_MOSI_PIN              GPIO_PIN_7      // PA7 - SPI1_MOSI

/* W5500 Control Pins */
#define W5500_CS_PORT               GPIOA
#define W5500_CS_PIN                GPIO_PIN_4      // PA4 - Chip Select (NSS)

#define W5500_RST_PORT              GPIOB
#define W5500_RST_PIN               GPIO_PIN_0      // PB0 - Reset (Optional)

#define W5500_INT_PORT              GPIOB
#define W5500_INT_PIN               GPIO_PIN_1      // PB1 - Interrupt (Optional)

/* ============================================================================
 * GPIO Macros for W5500
 * ============================================================================ */

/* Chip Select Control */
#define W5500_CS_LOW()              (W5500_CS_PORT->BRR = W5500_CS_PIN)
#define W5500_CS_HIGH()             (W5500_CS_PORT->BSRR = W5500_CS_PIN)

/* Reset Control */
#define W5500_RST_LOW()             (W5500_RST_PORT->BRR = W5500_RST_PIN)
#define W5500_RST_HIGH()            (W5500_RST_PORT->BSRR = W5500_RST_PIN)

/* ============================================================================
 * Register Bit Definitions (for bare-metal)
 * ============================================================================ */

/* GPIO Pin definitions */
#ifndef GPIO_PIN_0
#define GPIO_PIN_0                  ((uint16_t)0x0001)
#define GPIO_PIN_1                  ((uint16_t)0x0002)
#define GPIO_PIN_2                  ((uint16_t)0x0004)
#define GPIO_PIN_3                  ((uint16_t)0x0008)
#define GPIO_PIN_4                  ((uint16_t)0x0010)
#define GPIO_PIN_5                  ((uint16_t)0x0020)
#define GPIO_PIN_6                  ((uint16_t)0x0040)
#define GPIO_PIN_7                  ((uint16_t)0x0080)
#define GPIO_PIN_8                  ((uint16_t)0x0100)
#define GPIO_PIN_9                  ((uint16_t)0x0200)
#define GPIO_PIN_10                 ((uint16_t)0x0400)
#define GPIO_PIN_11                 ((uint16_t)0x0800)
#define GPIO_PIN_12                 ((uint16_t)0x1000)
#define GPIO_PIN_13                 ((uint16_t)0x2000)
#define GPIO_PIN_14                 ((uint16_t)0x4000)
#define GPIO_PIN_15                 ((uint16_t)0x8000)
#endif

/* ============================================================================
 * STM32F103 Peripheral Base Addresses
 * ============================================================================ */

#define PERIPH_BASE                 ((uint32_t)0x40000000)
#define APB1PERIPH_BASE             PERIPH_BASE
#define APB2PERIPH_BASE             (PERIPH_BASE + 0x10000)
#define AHBPERIPH_BASE              (PERIPH_BASE + 0x20000)

/* GPIO Base Addresses */
#define GPIOA_BASE                  (APB2PERIPH_BASE + 0x0800)
#define GPIOB_BASE                  (APB2PERIPH_BASE + 0x0C00)
#define GPIOC_BASE                  (APB2PERIPH_BASE + 0x1000)

/* SPI Base Addresses */
#define SPI1_BASE                   (APB2PERIPH_BASE + 0x3000)
#define SPI2_BASE                   (APB1PERIPH_BASE + 0x3800)

/* RCC Base Address */
#define RCC_BASE                    (AHBPERIPH_BASE + 0x1000)

/* FLASH Base Address */
#define FLASH_R_BASE                (AHBPERIPH_BASE + 0x2000)

/* ============================================================================
 * GPIO Register Structure
 * ============================================================================ */

typedef struct {
    volatile uint32_t CRL;          // Configuration Register Low
    volatile uint32_t CRH;          // Configuration Register High
    volatile uint32_t IDR;          // Input Data Register
    volatile uint32_t ODR;          // Output Data Register
    volatile uint32_t BSRR;         // Bit Set/Reset Register
    volatile uint32_t BRR;          // Bit Reset Register
    volatile uint32_t LCKR;         // Lock Register
} GPIO_TypeDef;

/* ============================================================================
 * SPI Register Structure
 * ============================================================================ */

typedef struct {
    volatile uint32_t CR1;          // Control Register 1
    volatile uint32_t CR2;          // Control Register 2
    volatile uint32_t SR;           // Status Register
    volatile uint32_t DR;           // Data Register
    volatile uint32_t CRCPR;        // CRC Polynomial Register
    volatile uint32_t RXCRCR;       // RX CRC Register
    volatile uint32_t TXCRCR;       // TX CRC Register
    volatile uint32_t I2SCFGR;      // I2S Configuration Register
    volatile uint32_t I2SPR;        // I2S Prescaler Register
} SPI_TypeDef;

/* ============================================================================
 * RCC Register Structure
 * ============================================================================ */

typedef struct {
    volatile uint32_t CR;           // Clock Control Register
    volatile uint32_t CFGR;         // Clock Configuration Register
    volatile uint32_t CIR;          // Clock Interrupt Register
    volatile uint32_t APB2RSTR;     // APB2 Peripheral Reset Register
    volatile uint32_t APB1RSTR;     // APB1 Peripheral Reset Register
    volatile uint32_t AHBENR;       // AHB Peripheral Clock Enable Register
    volatile uint32_t APB2ENR;      // APB2 Peripheral Clock Enable Register
    volatile uint32_t APB1ENR;      // APB1 Peripheral Clock Enable Register
    volatile uint32_t BDCR;         // Backup Domain Control Register
    volatile uint32_t CSR;          // Control/Status Register
} RCC_TypeDef;

/* ============================================================================
 * Peripheral Instances
 * ============================================================================ */

#define GPIOA                       ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB                       ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC                       ((GPIO_TypeDef *)GPIOC_BASE)
#define SPI1                        ((SPI_TypeDef *)SPI1_BASE)
#define SPI2                        ((SPI_TypeDef *)SPI2_BASE)
#define RCC                         ((RCC_TypeDef *)RCC_BASE)

/* ============================================================================
 * FLASH Register Structure
 * ============================================================================ */

typedef struct {
    volatile uint32_t ACR;          // Access Control Register
    volatile uint32_t KEYR;         // Key Register
    volatile uint32_t OPTKEYR;      // Option Key Register
    volatile uint32_t SR;           // Status Register
    volatile uint32_t CR;           // Control Register
    volatile uint32_t AR;           // Address Register
    volatile uint32_t RESERVED;
    volatile uint32_t OBR;          // Option Byte Register
    volatile uint32_t WRPR;         // Write Protection Register
} FLASH_TypeDef;

#define FLASH                       ((FLASH_TypeDef *)FLASH_R_BASE)

/* ============================================================================
 * RCC Clock Enable Bits
 * ============================================================================ */

#define RCC_APB2ENR_IOPAEN          ((uint32_t)0x00000004)  // GPIOA clock enable
#define RCC_APB2ENR_IOPBEN          ((uint32_t)0x00000008)  // GPIOB clock enable
#define RCC_APB2ENR_IOPCEN          ((uint32_t)0x00000010)  // GPIOC clock enable
#define RCC_APB2ENR_SPI1EN          ((uint32_t)0x00001000)  // SPI1 clock enable
#define RCC_APB2ENR_AFIOEN          ((uint32_t)0x00000001)  // AFIO clock enable

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

void W5500_GPIO_Init(void);
void W5500_SPI_Init(void);
void W5500_Reset(void);
uint8_t W5500_SPI_ReadWrite(uint8_t data);

#ifdef __cplusplus
}
#endif

#endif /* __W5500_CONF_H */
