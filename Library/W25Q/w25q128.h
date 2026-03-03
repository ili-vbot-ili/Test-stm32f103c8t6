/**
 ******************************************************************************
 * @file           : w25q128.h
 * @brief          : W25Q128 SPI Flash Driver Header (Software SPI)
 *                   Compatible with STM32F103C8T6
 ******************************************************************************
 * @attention
 *
 * W25Q128 Flash Memory (16MB)
 * - Interface: SPI (Software bit-bang, reusing P10 pins)
 * - Capacity: 128Mbit = 16MB
 * - Page size: 256 bytes
 * - Sector size: 4KB (smallest erase unit)
 * - Block size: 64KB
 * - Total sectors: 4096
 * - Endurance: 100,000 erase/program cycles
 *
 * Pin Configuration (reuses P10 pins on GPIOB):
 * - CS   : PB12 (was P10 Row B)
 * - CLK  : PB13 (was P10 CLK)
 * - MISO : PB14 (was P10 LAT) - Data OUT from Flash
 * - MOSI : PB15 (was P10 DATA) - Data IN to Flash
 *
 ******************************************************************************
 */

#ifndef __W25Q128_H
#define __W25Q128_H

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

#ifndef GPIOB_BASE
#define GPIOB_BASE                  (APB2PERIPH_BASE + 0x0C00)
#endif

#ifndef RCC_BASE
#define RCC_BASE                    (AHBPERIPH_BASE + 0x1000)
#endif

/* ============================================================================
 * GPIO Register Structure
 * ============================================================================ */

#ifndef __W25Q_GPIO_TYPEDEF__
#define __W25Q_GPIO_TYPEDEF__
typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} W25Q_GPIO_TypeDef;
#endif

#ifndef __W25Q_RCC_TYPEDEF__
#define __W25Q_RCC_TYPEDEF__
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
} W25Q_RCC_TypeDef;
#endif

/* ============================================================================
 * Pin Definitions (GPIOB)
 * ============================================================================ */

#define W25Q_CS_PIN                 12      // PB12 - Chip Select
#define W25Q_CLK_PIN                13      // PB13 - Clock
#define W25Q_MISO_PIN               14      // PB14 - Master In Slave Out
#define W25Q_MOSI_PIN               15      // PB15 - Master Out Slave In

#define W25Q_GPIO                   ((W25Q_GPIO_TypeDef *)GPIOB_BASE)
#define W25Q_RCC                    ((W25Q_RCC_TypeDef *)RCC_BASE)

/* Pin control macros */
#define W25Q_CS_LOW()               (W25Q_GPIO->BRR  = (1 << W25Q_CS_PIN))
#define W25Q_CS_HIGH()              (W25Q_GPIO->BSRR = (1 << W25Q_CS_PIN))
#define W25Q_CLK_LOW()              (W25Q_GPIO->BRR  = (1 << W25Q_CLK_PIN))
#define W25Q_CLK_HIGH()             (W25Q_GPIO->BSRR = (1 << W25Q_CLK_PIN))
#define W25Q_MOSI_LOW()             (W25Q_GPIO->BRR  = (1 << W25Q_MOSI_PIN))
#define W25Q_MOSI_HIGH()            (W25Q_GPIO->BSRR = (1 << W25Q_MOSI_PIN))
#define W25Q_MISO_READ()            ((W25Q_GPIO->IDR >> W25Q_MISO_PIN) & 1)

/* ============================================================================
 * W25Q128 Commands
 * ============================================================================ */

#define W25Q_CMD_WRITE_ENABLE       0x06
#define W25Q_CMD_WRITE_DISABLE      0x04
#define W25Q_CMD_READ_STATUS1       0x05
#define W25Q_CMD_READ_STATUS2       0x35
#define W25Q_CMD_WRITE_STATUS       0x01
#define W25Q_CMD_READ_DATA          0x03
#define W25Q_CMD_FAST_READ          0x0B
#define W25Q_CMD_PAGE_PROGRAM       0x02
#define W25Q_CMD_SECTOR_ERASE       0x20    // 4KB
#define W25Q_CMD_BLOCK_ERASE_32K    0x52    // 32KB
#define W25Q_CMD_BLOCK_ERASE_64K    0xD8    // 64KB
#define W25Q_CMD_CHIP_ERASE         0xC7
#define W25Q_CMD_POWER_DOWN         0xB9
#define W25Q_CMD_RELEASE_POWER      0xAB
#define W25Q_CMD_DEVICE_ID          0xAB
#define W25Q_CMD_JEDEC_ID           0x9F
#define W25Q_CMD_UNIQUE_ID          0x4B

/* Status register bits */
#define W25Q_STATUS_BUSY            0x01
#define W25Q_STATUS_WEL             0x02

/* ============================================================================
 * W25Q128 Parameters
 * ============================================================================ */

#define W25Q128_PAGE_SIZE           256
#define W25Q128_SECTOR_SIZE         4096        // 4KB
#define W25Q128_BLOCK_SIZE          65536       // 64KB
#define W25Q128_TOTAL_SIZE          (16 * 1024 * 1024)  // 16MB
#define W25Q128_NUM_SECTORS         4096
#define W25Q128_NUM_BLOCKS          256
#define W25Q128_NUM_PAGES           65536

/* Expected JEDEC ID for W25Q128 */
#define W25Q128_JEDEC_ID            0xEF4018

/* ============================================================================
 * Data Logger Configuration
 * ============================================================================ */

/* Log storage layout:
 * Sector 0 (0x000000-0x000FFF): Configuration & metadata
 *   - Bytes 0-3:   Magic number (0xDEADBEEF)
 *   - Bytes 4-7:   Log write index (next write position)
 *   - Bytes 8-11:  Log count
 *   - Bytes 12-15: Boot count
 *   - Bytes 16-31: Reserved
 *   - Bytes 32-63: Network config backup (IP, MAC, etc.)
 * 
 * Sectors 1-4095: Log data
 *   - Each log entry: 64 bytes
 *   - Max entries per sector: 64 (4096/64)
 *   - Total max entries: 64 * 4095 = 262,080 entries
 */

#define W25Q_LOG_MAGIC              0xDEADBEEF
#define W25Q_CONFIG_SECTOR          0           // Sector 0 for config
#define W25Q_LOG_START_SECTOR       1           // Log starts at sector 1
#define W25Q_LOG_ENTRY_SIZE         64          // Each log entry = 64 bytes
#define W25Q_LOG_ENTRIES_PER_SECTOR (W25Q128_SECTOR_SIZE / W25Q_LOG_ENTRY_SIZE)
#define W25Q_LOG_MAX_ENTRIES        (W25Q_LOG_ENTRIES_PER_SECTOR * (W25Q128_NUM_SECTORS - 1))

/* Log entry structure */
typedef struct {
    uint32_t timestamp;         // Relative timestamp (ms since boot)
    uint8_t  type;              // Log type: 0=TCP, 1=System, 2=Error
    uint8_t  dataLen;           // Length of data
    uint8_t  reserved[2];       // Alignment
    uint8_t  data[56];          // Log data (max 56 bytes)
} W25Q_LogEntry_t;

/* Log types */
#define W25Q_LOG_TCP_RECV           0
#define W25Q_LOG_TCP_SEND           1
#define W25Q_LOG_SYSTEM             2
#define W25Q_LOG_ERROR              3
#define W25Q_LOG_BOOT               4

/* ============================================================================
 * Public Functions
 * ============================================================================ */

/**
 * @brief  Initialize W25Q128 (GPIO + verify chip)
 * @retval 1 = OK, 0 = Failed (chip not detected)
 */
uint8_t W25Q_Init(void);

/**
 * @brief  Read JEDEC ID (Manufacturer + Device ID)
 * @retval 24-bit JEDEC ID (e.g. 0xEF4018 for W25Q128)
 */
uint32_t W25Q_ReadJEDEC(void);

/**
 * @brief  Read Device ID
 * @retval 8-bit device ID
 */
uint8_t W25Q_ReadDeviceID(void);

/**
 * @brief  Read data from flash
 * @param  addr: Start address (0 - 16MB)
 * @param  pBuf: Buffer to store data
 * @param  len: Number of bytes to read
 */
void W25Q_Read(uint32_t addr, uint8_t *pBuf, uint32_t len);

/**
 * @brief  Write data to flash (auto page-program, must erase first!)
 * @param  addr: Start address
 * @param  pBuf: Data to write
 * @param  len: Number of bytes to write (max 256 per page)
 */
void W25Q_WritePage(uint32_t addr, const uint8_t *pBuf, uint16_t len);

/**
 * @brief  Write data across multiple pages
 * @param  addr: Start address
 * @param  pBuf: Data to write
 * @param  len: Number of bytes to write
 */
void W25Q_Write(uint32_t addr, const uint8_t *pBuf, uint32_t len);

/**
 * @brief  Erase a 4KB sector
 * @param  sectorAddr: Any address within the sector
 */
void W25Q_EraseSector(uint32_t sectorAddr);

/**
 * @brief  Erase a 64KB block
 * @param  blockAddr: Any address within the block
 */
void W25Q_EraseBlock(uint32_t blockAddr);

/**
 * @brief  Erase entire chip (takes 20-100 seconds!)
 */
void W25Q_EraseChip(void);

/**
 * @brief  Check if chip is busy (writing/erasing)
 * @retval 1 = busy, 0 = ready
 */
uint8_t W25Q_IsBusy(void);

/**
 * @brief  Wait until chip is ready
 */
void W25Q_WaitBusy(void);

/**
 * @brief  Power down the chip (low power mode)
 */
void W25Q_PowerDown(void);

/**
 * @brief  Wake up from power down
 */
void W25Q_WakeUp(void);

/* ============================================================================
 * Data Logger Functions
 * ============================================================================ */

/**
 * @brief  Initialize log system (read metadata from sector 0)
 * @retval 1 = OK, 0 = first use (formatted)
 */
uint8_t W25Q_LogInit(void);

/**
 * @brief  Write a log entry
 * @param  type: Log type (W25Q_LOG_TCP_RECV, etc.)
 * @param  data: Log data
 * @param  len: Data length (max 56 bytes)
 */
void W25Q_LogWrite(uint8_t type, const uint8_t *data, uint8_t len);

/**
 * @brief  Read a log entry by index
 * @param  index: Log entry index
 * @param  entry: Output buffer
 * @retval 1 = OK, 0 = invalid index
 */
uint8_t W25Q_LogRead(uint32_t index, W25Q_LogEntry_t *entry);

/**
 * @brief  Get total number of log entries
 * @retval Number of log entries stored
 */
uint32_t W25Q_LogCount(void);

/**
 * @brief  Get boot count
 * @retval Number of times the device has booted
 */
uint32_t W25Q_GetBootCount(void);

/**
 * @brief  Erase all logs (keeps config sector metadata)
 */
void W25Q_LogClear(void);

/**
 * @brief  Check if W25Q128 is detected
 * @retval 1 = connected, 0 = not found
 */
uint8_t W25Q_IsConnected(void);

#ifdef __cplusplus
}
#endif

#endif /* __W25Q128_H */
