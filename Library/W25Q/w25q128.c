/**
 ******************************************************************************
 * @file           : w25q128.c
 * @brief          : W25Q128 SPI Flash Driver (Software SPI on GPIOB)
 ******************************************************************************
 * @attention
 *
 * Uses Software SPI (bit-bang) on PB12-PB15
 * Reuses P10 LED Matrix pins (P10 disabled)
 *
 * Pin mapping:
 *   PB12 = CS   (Output, Push-Pull)
 *   PB13 = CLK  (Output, Push-Pull)
 *   PB14 = MISO (Input, Pull-Up)
 *   PB15 = MOSI (Output, Push-Pull)
 *
 ******************************************************************************
 */

#include "w25q128.h"
#include <string.h>

/* ============================================================================
 * Internal Variables
 * ============================================================================ */

static uint8_t  _w25q_connected = 0;
static uint32_t _log_write_index = 0;   // Next write address offset in log area
static uint32_t _log_count = 0;
static uint32_t _boot_count = 0;
static uint32_t _tick_counter = 0;      // Simple tick counter for timestamps

/* ============================================================================
 * Software SPI (Bit-Bang)
 * ============================================================================ */

/**
 * @brief  Short delay for SPI timing (~100ns at 72MHz)
 */
static inline void _spi_delay(void)
{
    volatile uint8_t i = 2;
    while(i--);
}

/**
 * @brief  Transfer one byte via Software SPI (Mode 0: CPOL=0, CPHA=0)
 * @param  txData: Byte to send
 * @retval Byte received
 */
static uint8_t _spi_transfer(uint8_t txData)
{
    uint8_t rxData = 0;
    
    for(uint8_t bit = 0; bit < 8; bit++) {
        /* Set MOSI */
        if(txData & 0x80) {
            W25Q_MOSI_HIGH();
        } else {
            W25Q_MOSI_LOW();
        }
        txData <<= 1;
        
        /* Clock rising edge - sample MISO */
        W25Q_CLK_HIGH();
        _spi_delay();
        
        rxData <<= 1;
        if(W25Q_MISO_READ()) {
            rxData |= 1;
        }
        
        /* Clock falling edge */
        W25Q_CLK_LOW();
        _spi_delay();
    }
    
    return rxData;
}

/**
 * @brief  Send command + 24-bit address
 */
static void _send_cmd_addr(uint8_t cmd, uint32_t addr)
{
    W25Q_CS_LOW();
    _spi_transfer(cmd);
    _spi_transfer((addr >> 16) & 0xFF);
    _spi_transfer((addr >> 8) & 0xFF);
    _spi_transfer(addr & 0xFF);
}

/* ============================================================================
 * Initialization
 * ============================================================================ */

uint8_t W25Q_Init(void)
{
    /* Enable GPIOB clock */
    W25Q_RCC->APB2ENR |= (1 << 3);  // IOPBEN
    
    /* Configure PB12 (CS): Output Push-Pull, 50MHz */
    W25Q_GPIO->CRH &= ~(0x0F << 16);   // Clear PB12
    W25Q_GPIO->CRH |= (0x03 << 16);    // Output 50MHz, Push-Pull
    
    /* Configure PB13 (CLK): Output Push-Pull, 50MHz */
    W25Q_GPIO->CRH &= ~(0x0F << 20);   // Clear PB13
    W25Q_GPIO->CRH |= (0x03 << 20);    // Output 50MHz, Push-Pull
    
    /* Configure PB14 (MISO): Input Pull-Up */
    W25Q_GPIO->CRH &= ~(0x0F << 24);   // Clear PB14
    W25Q_GPIO->CRH |= (0x08 << 24);    // Input with Pull-Up/Pull-Down
    W25Q_GPIO->ODR |= (1 << 14);       // Pull-Up
    
    /* Configure PB15 (MOSI): Output Push-Pull, 50MHz */
    W25Q_GPIO->CRH &= ~(0x0F << 28);   // Clear PB15
    W25Q_GPIO->CRH |= (0x03 << 28);    // Output 50MHz, Push-Pull
    
    /* Set initial states */
    W25Q_CS_HIGH();
    W25Q_CLK_LOW();
    W25Q_MOSI_LOW();
    
    /* Small delay for power-up */
    for(volatile uint32_t i = 0; i < 10000; i++);
    
    /* Wake up chip (in case it was in power-down) */
    W25Q_WakeUp();
    
    /* Verify JEDEC ID */
    uint32_t jedec = W25Q_ReadJEDEC();
    
    /* Accept any Winbond W25Q flash:
     * W25Q128: 0xEF4018
     * W25Q64:  0xEF4017
     * W25Q32:  0xEF4016
     * Or other manufacturers */
    if((jedec >> 16) == 0xEF || (jedec >> 16) == 0xC8 || 
       (jedec >> 16) == 0x1C || (jedec >> 16) == 0x68) {
        _w25q_connected = 1;
    } else if(jedec != 0x000000 && jedec != 0xFFFFFF) {
        /* Unknown but valid ID - still usable */
        _w25q_connected = 1;
    } else {
        _w25q_connected = 0;
    }
    
    return _w25q_connected;
}

uint8_t W25Q_IsConnected(void)
{
    return _w25q_connected;
}

/* ============================================================================
 * Basic Flash Operations
 * ============================================================================ */

uint32_t W25Q_ReadJEDEC(void)
{
    uint32_t id = 0;
    
    W25Q_CS_LOW();
    _spi_transfer(W25Q_CMD_JEDEC_ID);
    id  = (uint32_t)_spi_transfer(0xFF) << 16;
    id |= (uint32_t)_spi_transfer(0xFF) << 8;
    id |= (uint32_t)_spi_transfer(0xFF);
    W25Q_CS_HIGH();
    
    return id;
}

uint8_t W25Q_ReadDeviceID(void)
{
    uint8_t id;
    
    W25Q_CS_LOW();
    _spi_transfer(W25Q_CMD_DEVICE_ID);
    _spi_transfer(0x00);  // Dummy
    _spi_transfer(0x00);  // Dummy
    _spi_transfer(0x00);  // Dummy
    id = _spi_transfer(0xFF);
    W25Q_CS_HIGH();
    
    return id;
}

static uint8_t _read_status1(void)
{
    uint8_t status;
    
    W25Q_CS_LOW();
    _spi_transfer(W25Q_CMD_READ_STATUS1);
    status = _spi_transfer(0xFF);
    W25Q_CS_HIGH();
    
    return status;
}

uint8_t W25Q_IsBusy(void)
{
    return (_read_status1() & W25Q_STATUS_BUSY) ? 1 : 0;
}

void W25Q_WaitBusy(void)
{
    while(_read_status1() & W25Q_STATUS_BUSY);
}

static void _write_enable(void)
{
    W25Q_CS_LOW();
    _spi_transfer(W25Q_CMD_WRITE_ENABLE);
    W25Q_CS_HIGH();
}

/* ============================================================================
 * Read / Write / Erase
 * ============================================================================ */

void W25Q_Read(uint32_t addr, uint8_t *pBuf, uint32_t len)
{
    W25Q_WaitBusy();
    
    _send_cmd_addr(W25Q_CMD_READ_DATA, addr);
    
    for(uint32_t i = 0; i < len; i++) {
        pBuf[i] = _spi_transfer(0xFF);
    }
    
    W25Q_CS_HIGH();
}

void W25Q_WritePage(uint32_t addr, const uint8_t *pBuf, uint16_t len)
{
    if(len == 0 || len > W25Q128_PAGE_SIZE) return;
    
    W25Q_WaitBusy();
    _write_enable();
    
    _send_cmd_addr(W25Q_CMD_PAGE_PROGRAM, addr);
    
    for(uint16_t i = 0; i < len; i++) {
        _spi_transfer(pBuf[i]);
    }
    
    W25Q_CS_HIGH();
    W25Q_WaitBusy();
}

void W25Q_Write(uint32_t addr, const uint8_t *pBuf, uint32_t len)
{
    uint32_t pageOffset;
    uint16_t pageRemain;
    uint32_t written = 0;
    
    while(written < len) {
        pageOffset = (addr + written) % W25Q128_PAGE_SIZE;
        pageRemain = W25Q128_PAGE_SIZE - pageOffset;
        
        uint16_t toWrite = (len - written > pageRemain) ? pageRemain : (uint16_t)(len - written);
        
        W25Q_WritePage(addr + written, pBuf + written, toWrite);
        written += toWrite;
    }
}

void W25Q_EraseSector(uint32_t sectorAddr)
{
    /* Align to sector boundary */
    sectorAddr &= ~(W25Q128_SECTOR_SIZE - 1);
    
    W25Q_WaitBusy();
    _write_enable();
    
    _send_cmd_addr(W25Q_CMD_SECTOR_ERASE, sectorAddr);
    W25Q_CS_HIGH();
    
    W25Q_WaitBusy();
}

void W25Q_EraseBlock(uint32_t blockAddr)
{
    blockAddr &= ~(W25Q128_BLOCK_SIZE - 1);
    
    W25Q_WaitBusy();
    _write_enable();
    
    _send_cmd_addr(W25Q_CMD_BLOCK_ERASE_64K, blockAddr);
    W25Q_CS_HIGH();
    
    W25Q_WaitBusy();
}

void W25Q_EraseChip(void)
{
    W25Q_WaitBusy();
    _write_enable();
    
    W25Q_CS_LOW();
    _spi_transfer(W25Q_CMD_CHIP_ERASE);
    W25Q_CS_HIGH();
    
    W25Q_WaitBusy();
}

void W25Q_PowerDown(void)
{
    W25Q_CS_LOW();
    _spi_transfer(W25Q_CMD_POWER_DOWN);
    W25Q_CS_HIGH();
    
    /* Wait tDP (3us) */
    for(volatile uint32_t i = 0; i < 300; i++);
}

void W25Q_WakeUp(void)
{
    W25Q_CS_LOW();
    _spi_transfer(W25Q_CMD_RELEASE_POWER);
    W25Q_CS_HIGH();
    
    /* Wait tRES1 (3us) */
    for(volatile uint32_t i = 0; i < 300; i++);
}

/* ============================================================================
 * Data Logger Implementation
 * ============================================================================ */

/* Config sector layout */
#define CONFIG_MAGIC_OFFSET         0
#define CONFIG_WRITE_IDX_OFFSET     4
#define CONFIG_LOG_COUNT_OFFSET     8
#define CONFIG_BOOT_COUNT_OFFSET    12

/**
 * @brief  Save metadata to config sector
 */
static void _save_config(void)
{
    uint8_t buf[32];
    
    /* Build config data */
    uint32_t magic = W25Q_LOG_MAGIC;
    memcpy(&buf[CONFIG_MAGIC_OFFSET], &magic, 4);
    memcpy(&buf[CONFIG_WRITE_IDX_OFFSET], &_log_write_index, 4);
    memcpy(&buf[CONFIG_LOG_COUNT_OFFSET], &_log_count, 4);
    memcpy(&buf[CONFIG_BOOT_COUNT_OFFSET], &_boot_count, 4);
    memset(&buf[16], 0xFF, 16);  // Reserved
    
    /* Erase config sector and write */
    W25Q_EraseSector(0);
    W25Q_WritePage(0, buf, 32);
}

uint8_t W25Q_LogInit(void)
{
    if(!_w25q_connected) return 0;
    
    /* Read config sector */
    uint8_t buf[32];
    W25Q_Read(0, buf, 32);
    
    uint32_t magic;
    memcpy(&magic, &buf[CONFIG_MAGIC_OFFSET], 4);
    
    if(magic == W25Q_LOG_MAGIC) {
        /* Valid config - restore state */
        memcpy(&_log_write_index, &buf[CONFIG_WRITE_IDX_OFFSET], 4);
        memcpy(&_log_count, &buf[CONFIG_LOG_COUNT_OFFSET], 4);
        memcpy(&_boot_count, &buf[CONFIG_BOOT_COUNT_OFFSET], 4);
        _boot_count++;
        _save_config();
        return 1;
    } else {
        /* First use - format */
        _log_write_index = 0;
        _log_count = 0;
        _boot_count = 1;
        _save_config();
        return 0;
    }
}

void W25Q_LogWrite(uint8_t type, const uint8_t *data, uint8_t len)
{
    if(!_w25q_connected) return;
    if(len > 56) len = 56;
    
    /* Calculate address in log area */
    uint32_t logAreaSize = (uint32_t)(W25Q128_NUM_SECTORS - 1) * W25Q128_SECTOR_SIZE;
    uint32_t logOffset = (_log_write_index * W25Q_LOG_ENTRY_SIZE) % logAreaSize;
    uint32_t addr = W25Q_LOG_START_SECTOR * W25Q128_SECTOR_SIZE + logOffset;
    
    /* Check if we need to erase a new sector */
    if((logOffset % W25Q128_SECTOR_SIZE) == 0) {
        W25Q_EraseSector(addr);
    }
    
    /* Build log entry */
    W25Q_LogEntry_t entry;
    entry.timestamp = _tick_counter;
    entry.type = type;
    entry.dataLen = len;
    entry.reserved[0] = 0;
    entry.reserved[1] = 0;
    memset(entry.data, 0, 56);
    if(data != 0 && len > 0) {
        memcpy(entry.data, data, len);
    }
    
    /* Write entry */
    W25Q_WritePage(addr, (const uint8_t *)&entry, W25Q_LOG_ENTRY_SIZE);
    
    /* Update metadata */
    _log_write_index++;
    _log_count++;
    
    /* Wrap around */
    if(_log_write_index >= W25Q_LOG_MAX_ENTRIES) {
        _log_write_index = 0;
    }
    
    /* Save config every 64 entries (once per sector) to reduce wear */
    if((_log_count % W25Q_LOG_ENTRIES_PER_SECTOR) == 0) {
        _save_config();
    }
}

uint8_t W25Q_LogRead(uint32_t index, W25Q_LogEntry_t *entry)
{
    if(!_w25q_connected || index >= _log_count) return 0;
    
    /* If we have more entries than max, adjust to read from circular buffer */
    uint32_t actualIndex;
    if(_log_count > W25Q_LOG_MAX_ENTRIES) {
        /* Circular buffer - oldest entry is at _log_write_index */
        actualIndex = (_log_write_index + index) % W25Q_LOG_MAX_ENTRIES;
    } else {
        actualIndex = index;
    }
    
    uint32_t addr = W25Q_LOG_START_SECTOR * W25Q128_SECTOR_SIZE 
                  + actualIndex * W25Q_LOG_ENTRY_SIZE;
    
    W25Q_Read(addr, (uint8_t *)entry, W25Q_LOG_ENTRY_SIZE);
    
    return 1;
}

uint32_t W25Q_LogCount(void)
{
    if(_log_count > W25Q_LOG_MAX_ENTRIES) {
        return W25Q_LOG_MAX_ENTRIES;
    }
    return _log_count;
}

uint32_t W25Q_GetBootCount(void)
{
    return _boot_count;
}

void W25Q_LogClear(void)
{
    _log_write_index = 0;
    _log_count = 0;
    /* Keep boot_count */
    _save_config();
}

/**
 * @brief  Update tick counter (call from main loop or timer)
 */
void W25Q_TickUpdate(uint32_t ms)
{
    _tick_counter += ms;
}
