/**
 ******************************************************************************
 * @file           : w5500_conf.c
 * @brief          : W5500 Hardware Configuration Implementation
 ******************************************************************************
 */

#include "w5500_conf.h"

/**
 * @brief  Initialize GPIO pins for W5500
 * @note   PA4: CS, PA5: SCK, PA6: MISO, PA7: MOSI, PB0: RST, PB1: INT
 */
void W5500_GPIO_Init(void)
{
    /* Enable GPIOA, GPIOB and AFIO clock */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
    
    /* Configure PA4 (CS) as Output Push-Pull, 50MHz */
    GPIOA->CRL &= ~(0x0F << 16);        // Clear PA4 config
    GPIOA->CRL |= (0x03 << 16);         // Output 50MHz, Push-Pull
    W5500_CS_HIGH();                     // CS idle high
    
    /* Configure PA5 (SCK) as Alternate Function Push-Pull, 50MHz */
    GPIOA->CRL &= ~(0x0F << 20);        // Clear PA5 config
    GPIOA->CRL |= (0x0B << 20);         // AF Output 50MHz, Push-Pull
    
    /* Configure PA6 (MISO) as Input Floating */
    GPIOA->CRL &= ~(0x0F << 24);        // Clear PA6 config
    GPIOA->CRL |= (0x04 << 24);         // Input Floating
    
    /* Configure PA7 (MOSI) as Alternate Function Push-Pull, 50MHz */
    GPIOA->CRL &= ~(0x0F << 28);        // Clear PA7 config
    GPIOA->CRL |= (0x0B << 28);         // AF Output 50MHz, Push-Pull
    
    /* Configure PB0 (RST) as Output Push-Pull, 50MHz */
    GPIOB->CRL &= ~(0x0F << 0);         // Clear PB0 config
    GPIOB->CRL |= (0x03 << 0);          // Output 50MHz, Push-Pull
    W5500_RST_HIGH();                    // RST idle high
    
    /* Configure PB1 (INT) as Input Pull-Up */
    GPIOB->CRL &= ~(0x0F << 4);         // Clear PB1 config
    GPIOB->CRL |= (0x08 << 4);          // Input with Pull-up/Pull-down
    GPIOB->ODR |= GPIO_PIN_1;           // Enable Pull-up
}

/**
 * @brief  Initialize SPI1 for W5500 communication
 * @note   SPI Mode 0 (CPOL=0, CPHA=0), MSB first, 8-bit
 */
void W5500_SPI_Init(void)
{
    /* Enable SPI1 clock */
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    
    /* Configure SPI1 */
    SPI1->CR1 = 0;
    
    /* SPI Configuration:
     * - Master mode
     * - Baud rate = fPCLK/8 (72MHz/8 = 9MHz)
     * - CPOL = 0 (Clock idle low)
     * - CPHA = 0 (Sample on first edge)
     * - 8-bit data frame
     * - MSB first
     * - Software slave management
     */
    SPI1->CR1 |= (1 << 2);              // Master mode
    SPI1->CR1 |= (0x02 << 3);           // Baud rate fPCLK/8
    SPI1->CR1 |= (1 << 8);              // SSI bit
    SPI1->CR1 |= (1 << 9);              // SSM - Software Slave Management
    
    /* Enable SPI1 */
    SPI1->CR1 |= (1 << 6);              // SPE - SPI Enable
}

/**
 * @brief  Reset W5500 via hardware reset pin
 */
void W5500_Reset(void)
{
    W5500_RST_LOW();
    /* Delay ~10ms (simple loop, adjust based on system clock) */
    for(volatile uint32_t i = 0; i < 720000; i++);
    W5500_RST_HIGH();
    /* Wait for W5500 to initialize */
    for(volatile uint32_t i = 0; i < 720000; i++);
}

/**
 * @brief  SPI Read/Write byte
 * @param  data: byte to send
 * @retval received byte
 */
uint8_t W5500_SPI_ReadWrite(uint8_t data)
{
    /* Wait until TX buffer empty */
    while(!(SPI1->SR & (1 << 1)));
    
    /* Send data */
    SPI1->DR = data;
    
    /* Wait until RX buffer not empty */
    while(!(SPI1->SR & (1 << 0)));
    
    /* Return received data */
    return (uint8_t)SPI1->DR;
}
