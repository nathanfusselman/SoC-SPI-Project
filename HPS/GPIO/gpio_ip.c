// GPIO IP Example
// GPIO IP Library (gpio_ip.c)
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: DE1-SoC Board

// Hardware configuration:
// GPIO Port:
//   GPIO_1[31-0] is used as a general purpose GPIO port
// HPS interface:
//   Mapped to offset of 0 in light-weight MM interface aperature
//   IRQ80 is used as the interrupt interface to the HPS

//-----------------------------------------------------------------------------

#include <stdint.h>          // C99 integer types -- uint32_t
#include <stdbool.h>         // bool
#include <fcntl.h>           // open
#include <sys/mman.h>        // mmap
#include <unistd.h>          // close
#include "../address_map.h"  // address map
#include "gpio_ip.h"         // gpio
#include "gpio_regs.h"       // registers

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

uint32_t *base = NULL;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

bool gpioOpen()
{
    // Open /dev/mem
    int file = open("/dev/mem", O_RDWR | O_SYNC);
    bool bOK = (file >= 0);
    if (bOK)
    {
        // Create a map from the physical memory location of
        // /dev/mem at an offset to LW avalon interface
        // with an aperature of SPAN_IN_BYTES bytes
        // to any location in the virtual 32-bit memory space of the process
        base = mmap(NULL, SPAN_IN_BYTES, PROT_READ | PROT_WRITE, MAP_SHARED,
                    file, LW_BRIDGE_BASE + GPIO_BASE_OFFSET);
        bOK = (base != MAP_FAILED);

        // Close /dev/mem
        close(file);
    }
    return bOK;
}

void selectPinPushPullOutput(uint8_t pin)
{
    uint32_t mask = 1 << pin;
    *(base+OFS_OD)  &= ~mask;
    *(base+OFS_OUT) |=  mask;
}

void selectPinOpenDrainOutput(uint8_t pin)
{
    uint32_t mask = 1 << pin;
    *(base+OFS_OD)  |= mask;
    *(base+OFS_OUT) |= mask;
}

void selectPinDigitalInput(uint8_t pin)
{
    uint32_t mask = 1 << pin;
    *(base+OFS_OUT) &= ~mask;
}

void selectPinInterruptRisingEdge(uint8_t pin)
{
    uint32_t mask = 1 << pin;
    *(base+OFS_INT_POSITIVE)  |=  mask;
    *(base+OFS_INT_NEGATIVE)  &= ~mask;
    *(base+OFS_INT_EDGE_MODE) |=  mask;
}

void selectPinInterruptFallingEdge(uint8_t pin)
{
    uint32_t mask = 1 << pin;
    *(base+OFS_INT_POSITIVE)  &= ~mask;
    *(base+OFS_INT_NEGATIVE)  |=  mask;
    *(base+OFS_INT_EDGE_MODE) |=  mask;
}

void selectPinInterruptBothEdges(uint8_t pin)
{
    uint32_t mask = 1 << pin;
    *(base+OFS_INT_POSITIVE)  |= mask;
    *(base+OFS_INT_NEGATIVE)  |= mask;
    *(base+OFS_INT_EDGE_MODE) |= mask;
}

void selectPinInterruptHighLevel(uint8_t pin)
{
    uint32_t mask = 1 << pin;
    *(base+OFS_INT_POSITIVE)  |=  mask;
    *(base+OFS_INT_NEGATIVE)  &= ~mask;
    *(base+OFS_INT_EDGE_MODE) &= ~mask;
}

void selectPinInterruptLowLevel(uint8_t pin)
{
    uint32_t mask = 1 << pin;
    *(base+OFS_INT_POSITIVE)  &= ~mask;
    *(base+OFS_INT_NEGATIVE)  |=  mask;
    *(base+OFS_INT_EDGE_MODE) &= ~mask;
}

void enablePinInterrupt(uint8_t pin)
{
    uint32_t mask = 1 << pin;
    *(base+OFS_INT_ENABLE) |= mask;
}

void disablePinInterrupt(uint8_t pin)
{
    uint32_t mask = 1 << pin;
    *(base+OFS_INT_ENABLE) &= ~mask;
}

void setPinValue(uint8_t pin, bool value)
{
    uint32_t mask = 1 << pin;
    if (value)
        *(base+OFS_DATA) |= mask;
    else
        *(base+OFS_DATA) &= ~mask;
}

bool getPinValue(uint8_t pin)
{
    uint32_t value = *(base+OFS_DATA);
    return (value >> pin) & 1;
}

void setPortValue(uint32_t value)
{
     *(base+OFS_DATA) = value;
}

uint32_t getPortValue()
{
    uint32_t value = *(base+OFS_DATA);
    return value;
}
