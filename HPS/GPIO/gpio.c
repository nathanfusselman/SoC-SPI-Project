// GPIO IP Example, gpio.c
// GPIO Shell Command
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: DE1-SoC Board

// Hardware configuration:
// GPIO Port:
//   GPIO_1[31-0] is used as a general purpose GPIO port
// IRQ:
//   IRQ80 is used as the interrupt interface to the HPS

//-----------------------------------------------------------------------------

#include <stdlib.h>          // EXIT_ codes
#include <stdio.h>           // printf
#include "gpio_ip.h"         // GPIO IP library

int main(int argc, char* argv[])
{
    int pin;
    
    if (argc == 2) {
        if ((strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
            printf("  usage:\n");
            printf("  gpio PIN out           make the pin a push-pull output\n");
            printf("  gpio PIN od            make the pin an open drain output\n");
            printf("  gpio PIN in            make the pin an input\n");
            printf("  \n");
            printf("  gpio PIN high          set the pin high\n");
            printf("  gpio PIN low           set the pin low\n");
            printf("  gpio PIN status        get the pin status\n");
            printf("  \n");
            printf("  gpio PIN int pos_edge  positive-edge interrupt\n");
            printf("  gpio PIN int neg_edge  negative-edge interrupt\n");
            printf("  gpio PIN int both_edge both-edge interrupt\n");
            printf("  gpio PIN int high      high-level interrupt\n");
            printf("  gpio PIN int low       low-level interrupt\n");
            printf("  gpio PIN int off       disable interrupt\n");
            printf("  \n");
            printf("  where PIN = [0, 31]\n");
        } else {
            printf("  command not understood\n");
        }
    } else if (argc == 3) {
        gpioOpen();
        pin = atoi(argv[1]);
        if (strcmp(argv[2], "out") == 0)
            selectPinPushPullOutput(pin);
        else if (strcmp(argv[2], "od") == 0)
            selectPinOpenDrainOutput(pin);
        else if (strcmp(argv[2], "in") == 0)
            selectPinDigitalInput(pin);
        else if (strcmp(argv[2], "high") == 0)
            setPinValue(pin, 1);
        else if (strcmp(argv[2], "low") == 0)
            setPinValue(pin, 0);
        else if (strcmp(argv[2], "status") == 0)
            if (getPinValue(pin))
                printf("pin was high (1)\n");
            else
                printf("pin was low (0)\n");
        else
            printf("argument %s not expected\n", argv[2]);
    } else if (argc == 4) {
        if (strcmp(argv[2], "int") == 0) {
            gpioOpen();
            pin = atoi(argv[1]);
            if (strcmp(argv[3], "pos_edge") == 0)
            {
                disablePinInterrupt(pin);
                selectPinInterruptRisingEdge(pin);
                enablePinInterrupt(pin);
            }
            else if (strcmp(argv[3], "neg_edge") == 0)
            {
                disablePinInterrupt(pin);
                selectPinInterruptFallingEdge(pin);
                enablePinInterrupt(pin);
            }
            else if (strcmp(argv[3], "both_edge") == 0)
            {
                disablePinInterrupt(pin);
                selectPinInterruptBothEdges(pin);
                enablePinInterrupt(pin);
            }
            else if (strcmp(argv[3], "high") == 0)
            {
                disablePinInterrupt(pin);
                selectPinInterruptHighLevel(pin);
                enablePinInterrupt(pin);
            }
            else if (strcmp(argv[3], "low") == 0)
            {
                disablePinInterrupt(pin);
                selectPinInterruptLowLevel(pin);
                enablePinInterrupt(pin);
            }
            else if (strcmp(argv[3], "off") == 0)
            {
                disablePinInterrupt(pin);
            }
            else
                printf("argument %s not expected\n", argv[3]);
        }
    } else {
        printf("  command not understood\n");
    }
    
    return EXIT_SUCCESS;
}

