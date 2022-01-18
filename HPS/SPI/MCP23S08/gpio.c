// SPI GPIO Expander
// SPI GPIO Expander CLI
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
// Device includes, defines, and assembler directives
//=============================================================================

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "gpio_expander.h"

//=============================================================================
// Subroutines
//=============================================================================

void printUint32InBinary(uint32_t n);

// Initialize Hardware
void initHw()
{
    // Initialize GPIO Expander
    gpioExpanderOpen();
}

//=============================================================================
// Main
//=============================================================================

int main(int argc, char* argv[])
{
    initHw();

    bool valid_command = false;

    if (argc > 1) {
            if ((strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) && argc == 2) {
                printf("  usage:\n");
                printf("  gpio [0-8] status                       Gets current PIN status\n");
                printf("  \n");
                printf("  gpio [0-8] dir                         Gets current PIN direction\n");
                printf("  gpio [0-8] dir set [input/output]      Sets current PIN direction\n");
                printf("  \n");
                printf("  gpio [0-8] pullup                      Gets current PIN pullup\n");
                printf("  gpio [0-8] pullup set [enable/disable] Sets current PIN pullup\n");
                printf("  \n");
                printf("  gpio [0-8] data                        Gets current PIN data\n");
                printf("  gpio [0-8] data set [on/off]           Sets current PIN data\n");
                valid_command = true;
            } else if (argv[1][0] >= (48 + 0) && argv[1][0] <= (48 + 8) && argc > 2) {
                uint8_t pin = (uint8_t)strtol(argv[1], NULL, 10);
                if (strcmp(argv[2], "status") == 0) {
                    if (argc == 3) {
                        // status print
                        printf("  Status: \n    dir: %s\n    pullup: %s\n    data: %s\n", getPinDir(pin) ? "INPUT" : "OUTPUT", getPinPullup(pin) ? "ENABLED" : "DISABLED", getPinValue(pin) ? "ON" : "OFF");
                        valid_command = true;
                    }
                } else if (strcmp(argv[2], "dir") == 0) {
                    if (argc == 3) {
                        // dir print
                        printf("  dir: %s\n", getPinDir(pin) ? "INPUT" : "OUTPUT");
                        valid_command = true;
                    } else if (argc == 5) {
                        if (strcmp(argv[3], "set") == 0 && strcmp(argv[4], "input") == 0) {
                            // dir set input
                            setPinDir(pin, DIR_INPUT);
                            printf("  dir set to INPUT\n");
                            valid_command = true;
                        } else if (strcmp(argv[3], "set") == 0 && strcmp(argv[4], "output") == 0) {
                            // dir set output
                            setPinDir(pin, DIR_OUTPUT);
                            printf("  dir set to OUTPUT\n");
                            valid_command = true;
                        }
                    }
                } else if (strcmp(argv[2], "pullup") == 0) {
                    if (argc == 3) {
                        printf("  pullup: %s\n", getPinPullup(pin) ? "ENABLED" : "DISABLED");
                        valid_command = true;
                    } else if (argc == 5) {
                        if (strcmp(argv[3], "set") == 0 && strcmp(argv[4], "enable") == 0) {
                            // pullup set enable
                            setPinPullup(pin, PULLUP_ENABLE);
                            printf("  pullup set to ENABLED\n");
                            valid_command = true;
                        } else if (strcmp(argv[3], "set") == 0 && strcmp(argv[4], "disable") == 0) {
                            // pullup set disable
                            setPinPullup(pin, PULLUP_DISABLE);
                            printf("  pullup set to DISABLED\n");
                            valid_command = true;
                        }
                    }
                } else if (strcmp(argv[2], "data") == 0) {
                    if (argc == 3) {
                        printf("  pin %d, data: %s\n", pin, getPinValue(pin) ? "ON" : "OFF");
                        valid_command = true;
                    } else if (argc == 5) {
                        if (strcmp(argv[3], "set") == 0 && strcmp(argv[4], "on") == 0) {
                            // data set on
                            setPinValue(pin, ON);
                            printf("  pin %d, data set to ON\n", pin);
                            valid_command = true;
                        } else if (strcmp(argv[3], "set") == 0 && strcmp(argv[4], "off") == 0) {
                            // data set off
                            setPinValue(pin, OFF);
                            printf("  pin %d, data set to OFF\n", pin);
                            valid_command = true;
                        }
                    }
                }
            }
        }
        if (!valid_command) {
            printf("  command not understood\n");
        }
}
