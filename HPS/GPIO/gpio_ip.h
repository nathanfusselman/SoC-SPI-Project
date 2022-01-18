// GPIO IP Example
// GPIO IP Library (gpio_ip.h)
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

#ifndef GPIO_H_
#define GPIO_H_

#include <stdint.h>
#include <stdbool.h>

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void selectPinPushPullOutput(uint8_t pin);
void selectPinOpenDrainOutput(uint8_t pin);
void selectPinDigitalInput(uint8_t pin);

void selectPinInterruptRisingEdge(uint8_t pin);
void selectPinInterruptFallingEdge(uint8_t pin);
void selectPinInterruptBothEdges(uint8_t pin);
void selectPinInterruptHighLevel(uint8_t pin);
void selectPinInterruptLowLevel(uint8_t pin);
void enablePinInterrupt(uint8_t pin);
void disablePinInterrupt(uint8_t pin);

void setPinValue(uint8_t pin, bool value);
bool getPinValue(uint8_t pin);
void setPortValue(uint32_t value);
uint32_t getPortValue();

#endif
