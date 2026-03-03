/**
 ******************************************************************************
 * @file           : wiegand.c
 * @brief          : Multi-Port Wiegand Protocol Reader Driver (Polling-based)
 ******************************************************************************
 * @attention
 *
 * 2-port polling-based Wiegand reader on GPIOA:
 *   Port 0: PA3 (D0), PA8  (D1) — Main reader
 *   Port 1: PA0 (D0), PA1  (D1) — Auxiliary 1
 *
 * JTAG is preserved (PA15/PB3/PB4 not touched).
 *
 * D0/D1 idle HIGH, pulse LOW ~50us for each bit
 * D0 pulse = '0' bit, D1 pulse = '1' bit
 *
 * Call WG_Process() frequently in main loop (every iteration)
 * When a complete frame is detected (25ms timeout), it auto-decodes.
 *
 ******************************************************************************
 */

#include "wiegand.h"

/* ============================================================================
 * Per-Port Internal State
 * ============================================================================ */

typedef struct {
    WG_State_t   state;
    uint64_t     bits;          // Bit accumulator
    uint8_t      bitCount;      // Bits received so far
    uint32_t     timeout;       // Timeout counter
    uint8_t      dataReady;     // Flag: card data available
    WG_CardData_t lastCard;     // Last decoded card
    uint8_t      prevD0;        // Previous D0 state (edge detection)
    uint8_t      prevD1;        // Previous D1 state
    uint8_t      d0Pin;         // D0 pin number
    uint8_t      d1Pin;         // D1 pin number
} WG_PortState_t;

static WG_PortState_t _ports[WG_NUM_PORTS];

/* ============================================================================
 * Helpers
 * ============================================================================ */

/**
 * @brief  Count number of '1' bits in value
 */
static uint8_t _popcount(uint32_t v)
{
    uint8_t count = 0;
    while(v) {
        count += (v & 1);
        v >>= 1;
    }
    return count;
}

/**
 * @brief  Integer to decimal string
 */
static void _uint_to_str(uint32_t val, char *buf)
{
    char tmp[12];
    int i = 0;
    
    if(val == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    
    while(val > 0) {
        tmp[i++] = '0' + (val % 10);
        val /= 10;
    }
    
    /* Reverse */
    for(int j = 0; j < i; j++) {
        buf[j] = tmp[i - 1 - j];
    }
    buf[i] = '\0';
}

/**
 * @brief  Read a GPIOA pin
 */
static uint8_t _readPin(uint8_t pin)
{
    return (WG_GPIO->IDR & (1 << pin)) ? 1 : 0;
}

/**
 * @brief  Configure a GPIOA pin as Input Pull-Up
 */
static void _configInputPullUp(uint8_t pin)
{
    if(pin < 8) {
        /* CRL: pins 0-7, each pin uses 4 bits */
        uint32_t shift = pin * 4;
        WG_GPIO->CRL &= ~(0x0FUL << shift);
        WG_GPIO->CRL |=  (0x08UL << shift);    // Input with Pull-Up/Down
    } else {
        /* CRH: pins 8-15, each pin uses 4 bits */
        uint32_t shift = (pin - 8) * 4;
        WG_GPIO->CRH &= ~(0x0FUL << shift);
        WG_GPIO->CRH |=  (0x08UL << shift);
    }
    WG_GPIO->ODR |= (1 << pin);    // Pull-Up (set ODR bit)
}

/* ============================================================================
 * Initialization
 * ============================================================================ */

void WG_InitPort(uint8_t port)
{
    if(port >= WG_NUM_PORTS) return;
    
    /* Enable GPIOA clock */
    WG_RCC->APB2ENR |= (1 << 2);  // IOPAEN
    
    /* Configure D0 and D1 pins as Input Pull-Up */
    _configInputPullUp(_ports[port].d0Pin);
    _configInputPullUp(_ports[port].d1Pin);
    
    /* Reset port state */
    WG_Reset(port);
}

void WG_Init(void)
{
    /* Setup pin mapping */
    _ports[0].d0Pin = WG0_D0_PIN;   // PA3
    _ports[0].d1Pin = WG0_D1_PIN;   // PA8
    _ports[1].d0Pin = WG1_D0_PIN;   // PA0
    _ports[1].d1Pin = WG1_D1_PIN;   // PA1
    
    /* Enable GPIOA clock */
    WG_RCC->APB2ENR |= (1 << 2);   // IOPAEN
    
    /* JTAG is preserved — PA15/PB3/PB4 not touched */
    
    /* Initialize all ports */
    for(uint8_t i = 0; i < WG_NUM_PORTS; i++) {
        WG_InitPort(i);
    }
}

/* ============================================================================
 * Per-Port Polling Process
 * ============================================================================ */

uint8_t WG_ProcessPort(uint8_t port)
{
    if(port >= WG_NUM_PORTS) return 0;
    
    WG_PortState_t *p = &_ports[port];
    
    uint8_t d0 = _readPin(p->d0Pin);
    uint8_t d1 = _readPin(p->d1Pin);
    
    switch(p->state)
    {
        case WG_STATE_IDLE:
            /* Detect falling edge on D0 or D1 */
            if(p->prevD0 == 1 && d0 == 0) {
                p->bits <<= 1;
                p->bitCount++;
                p->timeout = WG_TIMEOUT_LOOPS;
                p->state = WG_STATE_RECEIVING;
            }
            else if(p->prevD1 == 1 && d1 == 0) {
                p->bits <<= 1;
                p->bits |= 1;
                p->bitCount++;
                p->timeout = WG_TIMEOUT_LOOPS;
                p->state = WG_STATE_RECEIVING;
            }
            break;
            
        case WG_STATE_RECEIVING:
            /* Detect new bits (falling edges) */
            if(p->prevD0 == 1 && d0 == 0) {
                p->bits <<= 1;
                p->bitCount++;
                p->timeout = WG_TIMEOUT_LOOPS;
            }
            else if(p->prevD1 == 1 && d1 == 0) {
                p->bits <<= 1;
                p->bits |= 1;
                p->bitCount++;
                p->timeout = WG_TIMEOUT_LOOPS;
            }
            
            /* Check for max bits */
            if(p->bitCount >= WG_MAX_BITS) {
                p->state = WG_STATE_COMPLETE;
            }
            
            /* Timeout = frame complete */
            if(p->timeout > 0) {
                p->timeout--;
                if(p->timeout == 0 && p->bitCount > 0) {
                    p->state = WG_STATE_COMPLETE;
                }
            }
            break;
            
        case WG_STATE_COMPLETE:
            /* Decode the received data */
            if(p->bitCount >= 4) {
                WG_Decode(p->bits, p->bitCount, &p->lastCard);
                p->lastCard.port = port;
                p->dataReady = 1;
            }
            
            /* Reset for next card */
            p->bits = 0;
            p->bitCount = 0;
            p->timeout = 0;
            p->state = WG_STATE_IDLE;
            
            p->prevD0 = d0;
            p->prevD1 = d1;
            return 1;
            
        default:
            p->state = WG_STATE_IDLE;
            break;
    }
    
    p->prevD0 = d0;
    p->prevD1 = d1;
    return 0;
}

uint8_t WG_Process(void)
{
    uint8_t result = 0;
    for(uint8_t i = 0; i < WG_NUM_PORTS; i++) {
        if(WG_ProcessPort(i)) {
            result |= (1 << i);
        }
    }
    return result;
}

/* ============================================================================
 * Data Access
 * ============================================================================ */

uint8_t WG_GetCard(uint8_t port, WG_CardData_t *card)
{
    if(port >= WG_NUM_PORTS) return 0;
    if(!_ports[port].dataReady) return 0;
    
    *card = _ports[port].lastCard;
    _ports[port].dataReady = 0;
    return 1;
}

uint8_t WG_Available(uint8_t port)
{
    if(port >= WG_NUM_PORTS) return 0;
    return _ports[port].dataReady;
}

uint8_t WG_AnyAvailable(void)
{
    for(uint8_t i = 0; i < WG_NUM_PORTS; i++) {
        if(_ports[i].dataReady) return i;
    }
    return 0xFF;
}

void WG_Reset(uint8_t port)
{
    if(port >= WG_NUM_PORTS) return;
    
    WG_PortState_t *p = &_ports[port];
    p->state = WG_STATE_IDLE;
    p->bits = 0;
    p->bitCount = 0;
    p->timeout = 0;
    p->dataReady = 0;
    p->prevD0 = 1;
    p->prevD1 = 1;
    
    p->lastCard.cardNumber = 0;
    p->lastCard.facilityCode = 0;
    p->lastCard.rawData = 0;
    p->lastCard.rawBits = 0;
    p->lastCard.bitCount = 0;
    p->lastCard.format = WG_FORMAT_UNKNOWN;
    p->lastCard.valid = 0;
    p->lastCard.port = port;
}

void WG_ResetAll(void)
{
    for(uint8_t i = 0; i < WG_NUM_PORTS; i++) {
        WG_Reset(i);
    }
}

uint8_t WG_GetBitCount(uint8_t port)
{
    if(port >= WG_NUM_PORTS) return 0;
    return _ports[port].bitCount;
}

uint64_t WG_GetRawBits(uint8_t port)
{
    if(port >= WG_NUM_PORTS) return 0;
    return _ports[port].bits;
}

/* ============================================================================
 * Parity Check
 * ============================================================================ */

uint8_t WG_CheckParity26(uint32_t raw26)
{
    /*
     * Wiegand 26-bit: [P_even][8-bit FC][16-bit Card][P_odd]
     *   Bit 25 (MSB) = Even parity over bits 24..13 (upper 12 data bits)
     *   Bit 0  (LSB) = Odd parity over bits 12..1  (lower 12 data bits)
     */
    
    /* Even parity: bit25 + bits[24:13] should have even number of 1s */
    uint32_t upperHalf = (raw26 >> 13) & 0x1FFF;  // bits 25..13 (13 bits)
    if(_popcount(upperHalf) % 2 != 0) return 0;
    
    /* Odd parity: bits[12:0] should have odd number of 1s */
    uint32_t lowerHalf = raw26 & 0x1FFF;           // bits 12..0 (13 bits)
    if(_popcount(lowerHalf) % 2 != 1) return 0;
    
    return 1;
}

uint8_t WG_CheckParity34(uint64_t raw34)
{
    /*
     * Wiegand 34-bit: [P_even][16-bit FC][16-bit Card][P_odd]
     *   Bit 33 (MSB) = Even parity over bits 32..17 (upper 16 data bits)
     *   Bit 0  (LSB) = Odd parity over bits 16..1  (lower 16 data bits)
     */
    
    /* Even parity: bit33 + bits[32:17] should have even number of 1s */
    uint32_t upperHalf = (uint32_t)((raw34 >> 17) & 0x1FFFF);  // 17 bits
    if(_popcount(upperHalf) % 2 != 0) return 0;
    
    /* Odd parity: bits[16:0] should have odd number of 1s */
    uint32_t lowerHalf = (uint32_t)(raw34 & 0x1FFFF);          // 17 bits
    if(_popcount(lowerHalf) % 2 != 1) return 0;
    
    return 1;
}

/* ============================================================================
 * Decode
 * ============================================================================ */

uint8_t WG_Decode(uint64_t rawBits, uint8_t bitCount, WG_CardData_t *card)
{
    card->rawBits = rawBits;
    card->bitCount = bitCount;
    card->cardNumber = 0;
    card->facilityCode = 0;
    card->rawData = 0;
    card->format = WG_FORMAT_UNKNOWN;
    card->valid = 0;
    
    switch(bitCount)
    {
        case 26:
        {
            /*
             * Wiegand 26: [EP][8-bit FC][16-bit Card][OP]
             * Bits: 25=EP, 24..17=FC, 16..1=Card, 0=OP
             */
            uint32_t raw = (uint32_t)(rawBits & 0x03FFFFFF);
            
            card->format = WG_FORMAT_26BIT;
            card->facilityCode = (uint16_t)((raw >> 17) & 0xFF);
            card->cardNumber = (raw >> 1) & 0xFFFF;
            card->rawData = (raw >> 1) & 0xFFFFFF;  // 24 data bits (FC+Card)
            card->valid = WG_CheckParity26(raw);
            break;
        }
        
        case 34:
        {
            /*
             * Wiegand 34: [EP][16-bit FC][16-bit Card][OP]
             * Bits: 33=EP, 32..17=FC, 16..1=Card, 0=OP
             */
            uint64_t raw = rawBits & 0x3FFFFFFFFULL;
            
            card->format = WG_FORMAT_34BIT;
            card->facilityCode = (uint16_t)((raw >> 17) & 0xFFFF);
            card->cardNumber = (uint32_t)((raw >> 1) & 0xFFFF);
            card->rawData = (uint32_t)((raw >> 1) & 0xFFFFFFFF);
            card->valid = WG_CheckParity34(raw);
            break;
        }
        
        case 37:
        {
            /*
             * Wiegand 37: [EP][17-bit FC][18-bit Card][OP] (one common format)
             * Or: [EP][35-bit data][OP] (generic)
             * We'll decode as generic 35-bit data
             */
            uint64_t raw = rawBits & 0x1FFFFFFFFFULL;
            
            card->format = WG_FORMAT_37BIT;
            card->facilityCode = (uint16_t)((raw >> 19) & 0x1FFFF);  // 17 bits
            card->cardNumber = (uint32_t)((raw >> 1) & 0x3FFFF);     // 18 bits
            card->rawData = (uint32_t)((raw >> 1) & 0xFFFFFFFF);
            
            /* Simple parity check for 37-bit (same structure as 26/34) */
            uint32_t upperHalf = (uint32_t)((raw >> 19) & 0x7FFFF);  // 19 bits
            uint32_t lowerHalf = (uint32_t)(raw & 0x7FFFF);          // 19 bits
            card->valid = (_popcount(upperHalf) % 2 == 0) && 
                         (_popcount(lowerHalf) % 2 == 1) ? 1 : 0;
            break;
        }
        
        default:
        {
            /* Unknown format - store raw data */
            card->format = WG_FORMAT_UNKNOWN;
            card->rawData = (uint32_t)(rawBits & 0xFFFFFFFF);
            card->cardNumber = card->rawData;
            card->facilityCode = 0;
            card->valid = 0;  // Can't verify parity for unknown format
            break;
        }
    }
    
    return card->valid;
}

/* ============================================================================
 * Formatting
 * ============================================================================ */

void WG_FormatCard(const WG_CardData_t *card, char *buf)
{
    /* Format: "P0 FC:xxx ID:xxxxx" */
    int pos = 0;
    buf[pos++] = 'P';
    buf[pos++] = '0' + card->port;
    buf[pos++] = ' ';
    buf[pos++] = 'F';
    buf[pos++] = 'C';
    buf[pos++] = ':';
    char tmp[12];
    
    _uint_to_str(card->facilityCode, tmp);
    for(int i = 0; tmp[i]; i++) {
        buf[pos++] = tmp[i];
    }
    
    buf[pos++] = ' ';
    buf[pos++] = 'I';
    buf[pos++] = 'D';
    buf[pos++] = ':';
    
    _uint_to_str(card->cardNumber, tmp);
    for(int i = 0; tmp[i]; i++) {
        buf[pos++] = tmp[i];
    }
    
    buf[pos] = '\0';
}

void WG_FormatCardNumber(const WG_CardData_t *card, char *buf)
{
    _uint_to_str(card->cardNumber, buf);
}
