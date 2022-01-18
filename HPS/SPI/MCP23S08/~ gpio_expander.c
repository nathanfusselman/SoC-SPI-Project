// SPI GPIO Expander
// SPI GPIO Expander Library
// CSE4356-SoC | Fall 2021 | Term Project
// Deborah Jahaj and Nathan Fusselman

//=============================================================================
// Hardware Target
//=============================================================================

// Target Platform: DE1-SoC Board

// Hardware configuration:
// GPIO Port:
//   GPIO_1[31-0] is used as a general purpose GPIO port
// HPS interface:
//   Mapped to offset of 0 in light-weight MM interface aperature
//   IRQ80 is used as the interrupt interface to the HPS

//=============================================================================

#include <stdint.h>             // C99 integer types -- uint32_t
#include <stdbool.h>            // bool
#include <stdio.h>              // print
#include "gpio_expander.h"      // gpio expander
#include "../spi_ip.h"          // spi
#include "gpio_expander_regs.h" // registers
#include <unistd.h>

#define CLOCK_SPEED 5000000 // 5MHz
#define WORDSIZE 24 // 24-Bit

//=============================================================================
// Subroutines
//=============================================================================

bool gpioExpanderOpen()
{
    bool bOK = spiOpen(); // Create SPI Module
    if (bOK) bOK = setStatus(true); // Enable SPI
    if (bOK) bOK = setBRD(CLOCK_SPEED); // Set BRD
    if (bOK) bOK = setWordsize(WORDSIZE); // Set Wordsize
    return bOK;
}

void setPinDir(uint8_t pin, bool input)
{
    if (pin >= 8) return;
    uint32_t data = 0;
    sendData(READ | IODIR | BLANK);
    readData(&data);                           // Read Current Value
    if (input)
        data |= (1 << pin);
    else
        data &= ~(1 << pin);
    sendData(WRITE | IODIR | (data & 0xFF)); // Set New Value
    readData(&data);
}

bool getPinDir(uint8_t pin)
{
    if (pin >= 8) return;
    uint32_t data = 0;
    sendData(READ | IODIR | BLANK);
    readData(&data); // Read Current Value
    return (data >> pin) & 0x1;
}

void setPinPullup(uint8_t pin, bool value)
{
    if (pin >= 8) return;
    uint32_t data = 0;
    sendData(READ | GPPU | BLANK);
    readData(&data); // Read Current Value
    if (value) 
        data |= (1 << pin);
    else
        data &= ~(1 << pin);
    sendData(WRITE | GPPU | (data & 0xFF)); // Set New Value
    readData(&data);
}

bool getPinPullup(uint8_t pin)
{
    if (pin >= 8) return;
    uint32_t data = 0;
    sendData(READ | GPPU | BLANK);
    readData(&data); // Read Current Value
    return (data >> pin) & 0x1;
}

void setPinValue(uint8_t pin, bool value)
{
    if (pin >= 8) return;
    uint32_t data = 0;
    sendData(READ | GPIO | BLANK);
    readData(&data);                        // Read Current Value
    if (value) 
        data |= (1 << pin);
    else
        data &= ~(1 << pin);
    sendData(WRITE | GPIO | (data & 0xFF)); // Set New Value
    readData(&data);
}

bool getPinValue(uint8_t pin)
{
    if (pin >= 8) return;
    uint32_t data = 0;
    sendData(READ | GPIO | BLANK);
    readData(&data);                        // Read Current Value
    return ((data >> pin) & 0x1);
}
