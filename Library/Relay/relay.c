/**
 ******************************************************************************
 * @file           : relay.c
 * @brief          : 4-Channel Relay Control Driver Implementation
 ******************************************************************************
 * @attention
 *
 * 4 Relay outputs on GPIOA (PA9-PA12)
 * Push-Pull output, configurable active level (default: Active LOW)
 * Supports: individual control, bitmask, auto-off pulse, test sequence
 *
 ******************************************************************************
 */

#include "relay.h"

/* ============================================================================
 * Internal State
 * ============================================================================ */

static Relay_System_t _relay;

/* Pin number lookup table */
static const uint8_t _pinMap[RELAY_NUM_CHANNELS] = {
    RELAY0_PIN,     // PA9
    RELAY1_PIN,     // PA10
    RELAY2_PIN,     // PA11
    RELAY3_PIN      // PA12
};

/* ============================================================================
 * Internal Helpers
 * ============================================================================ */

/**
 * @brief  Set GPIO pin output level
 */
static void _setPin(uint8_t pin, uint8_t level)
{
    if(level) {
        RELAY_GPIO->BSRR = (1 << pin);     // Set HIGH
    } else {
        RELAY_GPIO->BRR = (1 << pin);       // Set LOW
    }
}

/**
 * @brief  Apply relay state to GPIO pin (handles active-low/high logic)
 */
static void _applyState(uint8_t ch)
{
    if(ch >= RELAY_NUM_CHANNELS) return;
    
#if RELAY_ACTIVE_HIGH
    /* Active HIGH: ON = pin HIGH, OFF = pin LOW */
    _setPin(_pinMap[ch], _relay.ch[ch].state);
#else
    /* Active LOW: ON = pin LOW, OFF = pin HIGH */
    _setPin(_pinMap[ch], !_relay.ch[ch].state);
#endif
}

/**
 * @brief  Simple delay (approximate ms)
 */
static void _relay_delay_ms(uint32_t ms)
{
    for(volatile uint32_t i = 0; i < ms * 7200; i++);
}

/* ============================================================================
 * Initialization
 * ============================================================================ */

void RELAY_Init(void)
{
    /* Enable GPIOA clock */
    RELAY_RCC->APB2ENR |= (1 << 2);  // IOPAEN
    
    /*
     * Configure PA9-PA12 as Output Push-Pull, 2MHz
     * PA9  = CRH bits [7:4]    → 0x2 (Output 2MHz, Push-Pull)
     * PA10 = CRH bits [11:8]   → 0x2
     * PA11 = CRH bits [15:12]  → 0x2
     * PA12 = CRH bits [19:16]  → 0x2
     */
    
    /* PA9: CRH bits [7:4] */
    RELAY_GPIO->CRH &= ~(0x0FUL << 4);
    RELAY_GPIO->CRH |=  (0x02UL << 4);     // Output 2MHz Push-Pull
    
    /* PA10: CRH bits [11:8] */
    RELAY_GPIO->CRH &= ~(0x0FUL << 8);
    RELAY_GPIO->CRH |=  (0x02UL << 8);
    
    /* PA11: CRH bits [15:12] */
    RELAY_GPIO->CRH &= ~(0x0FUL << 12);
    RELAY_GPIO->CRH |=  (0x02UL << 12);
    
    /* PA12: CRH bits [19:16] */
    RELAY_GPIO->CRH &= ~(0x0FUL << 16);
    RELAY_GPIO->CRH |=  (0x02UL << 16);
    
    /* Initialize all channels to OFF */
    for(uint8_t i = 0; i < RELAY_NUM_CHANNELS; i++) {
        _relay.ch[i].state = RELAY_OFF;
        _relay.ch[i].onTime = 0;
        _relay.ch[i].autoOffDelay = 0;
        _applyState(i);
    }
    
    _relay.initialized = 1;
}

/* ============================================================================
 * Individual Control
 * ============================================================================ */

void RELAY_On(uint8_t ch)
{
    if(ch >= RELAY_NUM_CHANNELS) return;
    _relay.ch[ch].state = RELAY_ON;
    _applyState(ch);
}

void RELAY_Off(uint8_t ch)
{
    if(ch >= RELAY_NUM_CHANNELS) return;
    _relay.ch[ch].state = RELAY_OFF;
    _relay.ch[ch].autoOffDelay = 0;
    _relay.ch[ch].onTime = 0;
    _applyState(ch);
}

void RELAY_Toggle(uint8_t ch)
{
    if(ch >= RELAY_NUM_CHANNELS) return;
    _relay.ch[ch].state = (_relay.ch[ch].state == RELAY_ON) ? RELAY_OFF : RELAY_ON;
    if(_relay.ch[ch].state == RELAY_OFF) {
        _relay.ch[ch].autoOffDelay = 0;
        _relay.ch[ch].onTime = 0;
    }
    _applyState(ch);
}

void RELAY_Set(uint8_t ch, uint8_t state)
{
    if(state) {
        RELAY_On(ch);
    } else {
        RELAY_Off(ch);
    }
}

uint8_t RELAY_Get(uint8_t ch)
{
    if(ch >= RELAY_NUM_CHANNELS) return RELAY_OFF;
    return _relay.ch[ch].state;
}

/* ============================================================================
 * Bulk Control
 * ============================================================================ */

void RELAY_AllOn(void)
{
    for(uint8_t i = 0; i < RELAY_NUM_CHANNELS; i++) {
        RELAY_On(i);
    }
}

void RELAY_AllOff(void)
{
    for(uint8_t i = 0; i < RELAY_NUM_CHANNELS; i++) {
        RELAY_Off(i);
    }
}

void RELAY_SetMask(uint8_t mask)
{
    for(uint8_t i = 0; i < RELAY_NUM_CHANNELS; i++) {
        RELAY_Set(i, (mask >> i) & 1);
    }
}

uint8_t RELAY_GetMask(void)
{
    uint8_t mask = 0;
    for(uint8_t i = 0; i < RELAY_NUM_CHANNELS; i++) {
        if(_relay.ch[i].state == RELAY_ON) {
            mask |= (1 << i);
        }
    }
    return mask;
}

/* ============================================================================
 * Pulse / Auto-Off
 * ============================================================================ */

void RELAY_Pulse(uint8_t ch, uint32_t delay)
{
    if(ch >= RELAY_NUM_CHANNELS) return;
    _relay.ch[ch].state = RELAY_ON;
    _relay.ch[ch].autoOffDelay = delay;
    _relay.ch[ch].onTime = 0;
    _applyState(ch);
}

void RELAY_Process(void)
{
    for(uint8_t i = 0; i < RELAY_NUM_CHANNELS; i++) {
        if(_relay.ch[i].state == RELAY_ON && _relay.ch[i].autoOffDelay > 0) {
            _relay.ch[i].onTime++;
            if(_relay.ch[i].onTime >= _relay.ch[i].autoOffDelay) {
                /* Auto-off expired */
                RELAY_Off(i);
            }
        }
    }
}

/* ============================================================================
 * Test & Utility
 * ============================================================================ */

void RELAY_TestAll(uint32_t delay_ms_val)
{
    for(uint8_t i = 0; i < RELAY_NUM_CHANNELS; i++) {
        RELAY_On(i);
        _relay_delay_ms(delay_ms_val);
        RELAY_Off(i);
        _relay_delay_ms(delay_ms_val / 2);
    }
}

uint8_t RELAY_IsInitialized(void)
{
    return _relay.initialized;
}

void RELAY_FormatStatus(char *buf)
{
    /* Format: "R:1010" — 1=ON, 0=OFF for each channel */
    buf[0] = 'R';
    buf[1] = ':';
    for(uint8_t i = 0; i < RELAY_NUM_CHANNELS; i++) {
        buf[2 + i] = (_relay.ch[i].state == RELAY_ON) ? '1' : '0';
    }
    buf[2 + RELAY_NUM_CHANNELS] = '\0';
}
