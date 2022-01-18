// SPI GPIO Expander
// User Test Example
// CSE4356-SoC | Fall 2021 | Term Project
// Deborah Jahaj and Nathan Fusselman

//=============================================================================
// Hardware Target
//=============================================================================

// Target Platform: DE1-SoC Board

// Hardware configuration:
//  HPS interface:
//      Mapped to offset of 0 in light-weight MM interface aperature
//  Red LED:
//      Pin0 drives a red LED through a 470 ohm resistror
//  Green LED:
//      Pin1 drives a green LED through a 470 ohm resistror
//  Pushbutton:
//      Pin2 internally pulled up

//=============================================================================
// Device includes, defines, and assembler directives
//=============================================================================

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "gpio_expander.h"

// Pins
#define BUTTON_0 0
#define RED_LED 1
#define ORANGE_LED 2
#define YELLOW_LED 3
#define BUTTON_1 4
#define BUTTON_2 5
#define GREEN_LED 6
#define BLUE_LED 7

//=============================================================================
// Subroutines
//=============================================================================

// Blocking function that returns only when BUTTON_0 is pressed
void waitPb0Press()
{
	while(getPinValue(BUTTON_0));
}

// Blocking function that returns only when BUTTON_1 is pressed
void waitPb1Press()
{
	while(getPinValue(BUTTON_1));
}

// Blocking function that returns only when BUTTON_2 is pressed
void waitPb2Press()
{
	while(getPinValue(BUTTON_2));
}

// Initialize Hardware
void initHw()
{
    // Initialize GPIO Expander
    gpioExpanderOpen();

    // Configure LED and pushbutton pins
    setPinDir(BUTTON_0, DIR_INPUT);
    setPinPullup(BUTTON_0, PULLUP_ENABLE);

    setPinDir(RED_LED, DIR_OUTPUT);
    setPinDir(ORANGE_LED, DIR_OUTPUT);
    setPinDir(YELLOW_LED, DIR_OUTPUT);

    setPinDir(BUTTON_1, DIR_INPUT);
    setPinPullup(BUTTON_1, PULLUP_ENABLE);

    setPinDir(BUTTON_2, DIR_INPUT);
    setPinPullup(BUTTON_2, PULLUP_ENABLE);

    setPinDir(GREEN_LED, DIR_OUTPUT);
    setPinDir(BLUE_LED, DIR_OUTPUT);

    setPinValue(RED_LED, OFF);
    setPinValue(ORANGE_LED, OFF);
    setPinValue(YELLOW_LED, OFF);
    setPinValue(GREEN_LED, OFF);
    setPinValue(BLUE_LED, OFF);
}

//=============================================================================
// Main
//=============================================================================

int main(void)
{
	// Initialize hardware
	initHw();

    while (true) {
        setPinValue(RED_LED, OFF);
        setPinValue(ORANGE_LED, OFF);
        setPinValue(YELLOW_LED, OFF);
        setPinValue(GREEN_LED, OFF);
        setPinValue(BLUE_LED, OFF);

        // Wait for PB0 press
        waitPb0Press();

        // Turn on red LED and all others off.
        setPinValue(RED_LED, ON);
        setPinValue(ORANGE_LED, OFF);
        setPinValue(YELLOW_LED, OFF);
        setPinValue(GREEN_LED, OFF);
        setPinValue(BLUE_LED, OFF);

        // Wait for PB1 press
        waitPb1Press();

        // Turn on orange LED and all others off.
        setPinValue(RED_LED, OFF);
        setPinValue(ORANGE_LED, ON);
        setPinValue(YELLOW_LED, OFF);
        setPinValue(GREEN_LED, OFF);
        setPinValue(BLUE_LED, OFF);

        // Wait for PB2 press
        waitPb2Press();

        // Turn on yellow LED and all others off.
        setPinValue(RED_LED, OFF);
        setPinValue(ORANGE_LED, OFF);
        setPinValue(YELLOW_LED, ON);
        setPinValue(GREEN_LED, OFF);
        setPinValue(BLUE_LED, OFF);

        // Wait for PB0 press
        waitPb0Press();

        // Turn on green LED and all others off.
        setPinValue(RED_LED, OFF);
        setPinValue(ORANGE_LED, OFF);
        setPinValue(YELLOW_LED, OFF);
        setPinValue(GREEN_LED, ON);
        setPinValue(BLUE_LED, OFF);

        // Wait for PB1 press
        waitPb1Press();

        // Turn on blue LED and all others off.
        setPinValue(RED_LED, OFF);
        setPinValue(ORANGE_LED, OFF);
        setPinValue(YELLOW_LED, OFF);
        setPinValue(GREEN_LED, OFF);
        setPinValue(BLUE_LED, ON);

        // Wait for PB1 press
        waitPb2Press();
    }
}
