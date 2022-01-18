// SPIGPIO Expander
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

#define DIR_INPUT 1
#define DIR_OUTPUT 0
#define PULLUP_ENABLE 1
#define PULLUP_DISABLE 0
#define ON 1
#define OFF 0

#ifndef GPIO_H_
#define GPIO_H_

#include <stdint.h>
#include <stdbool.h>

//=============================================================================
// Subroutines
//=============================================================================

bool gpioExpanderOpen(void);
void setPinDir(uint8_t pin, bool input);
bool getPinDir(uint8_t pin);
void setPinPullup(uint8_t pin, bool value);
bool getPinPullup(uint8_t pin);
void setPinValue(uint8_t pin, bool value);
bool getPinValue(uint8_t pin);

#endif
