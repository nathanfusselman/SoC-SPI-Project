// GPIO IP Example
// Stop_Go Application (stop_go.c)
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
// Red LED:
//   Pin GPIO_1_0 drives a red LED through a 470 ohm resistror
// Green LED:
//   Pin GPIO_1_1 drives a green LED through a 470 ohm resistror
// Pushbutton:
//   Pin GPIO_1_2 pulled low by switch, pulled high through 10k ohm resistor

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "gpio_ip.h"

// Pins
#define RED_LED 0
#define GREEN_LED 1
#define PUSH_BUTTON 2

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Blocking function that returns only when SW1 is pressed
void waitPbPress()
{
	while(getPinValue(PUSH_BUTTON));
}

// Initialize Hardware
void initHw()
{
    // Initialize GPIO IP
    gpioOpen();

    // Configure LED and pushbutton pins
    selectPinPushPullOutput(GREEN_LED);
    selectPinPushPullOutput(RED_LED);
    selectPinDigitalInput(PUSH_BUTTON);
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
	// Initialize hardware
	initHw();

    // Turn off green LED, turn on red LED
	setPinValue(GREEN_LED, 0);
    setPinValue(RED_LED, 1);

    // Wait for PB press
    waitPbPress();

    // Turn off red LED, turn on green LED
    setPinValue(RED_LED, 0);
    setPinValue(GREEN_LED, 1);
}
