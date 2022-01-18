// SPI IP
// SPI CLI App
// CSE4356-SoC | Fall 2021 | Term Project
// Deborah Jahaj and Nathan Fusselman

//=============================================================================
// Hardware Target
//=============================================================================

// Target Platform: DE1-SoC Board

// Hardware configuration:
// SPI Port:
//   GPIO_0[7,9,11,13,15,17,19] are used as a SPI interface
// HPS interface:
//   Mapped to offset of 8000 in light-weight MM interface aperature

//=============================================================================
// Device includes, defines, and assembler directives
//=============================================================================

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "spi_ip.h"

//=============================================================================
// Subroutines
//=============================================================================

void printUint32InBinary(uint32_t n);

// Initialize Hardware
void initHw()
{
    // Initialize GPIO IP
    spiOpen();
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
            printf("  spi status                             Gets current SPI status\n");
            printf("  spi status set [enable/disable]        Sets current SPI status\n");
            printf("  \n");
            printf("  spi send [data]                        Send data to Tx FIFO\n");
            printf("  spi read                               Read data from Rx FIFO\n");
            printf("  \n");
            printf("  spi [rx/tx] status                     Gets status of selected FIFO\n");
            printf("  spi [rx/tx] count                      Gets count of selected FIFO\n");
            printf("  spi [rx/tx] clearov                    Clear overflow for selected fifo\n");
            printf("  spi [rx/tx] reset                      Reset the selected fifo\n");
            printf("  \n");
            printf("  spi wordsize                           Gets current word size in bits\n");
            printf("  spi wordsize set [32-1]                Sets current word size in bits\n");
            printf("  \n");
            printf("  spi device                             Gets current selected device\n");
            printf("  spi device set [0-3]                   Sets current selected device\n");
            printf("  \n");
            printf("  spi [0-3] csmode                       Gets current mode for dev\n");
            printf("  spi [0-3] csmode set [auto/manual]     Sets current mode for dev\n");
            printf("  spi [0-3] cs                           Gets current cs state (manual)\n");
            printf("  spi [0-3] cs set [assert/deassert]     Sets current cs state (manual)\n");
            printf("  spi [0-3] mode                         Gets current device mode\n");
            printf("  spi [0-3] mode set [SPO] [SPH]         Sets current device mode\n");
            printf("  \n");
            printf("  spi brd                                Gets current baud rate\n");
            printf("  spi brd set [baud_rate]                Sets current baud rate\n");
            valid_command = true;
        } else if ((strcmp(argv[1], "status") == 0)) {
            if (argc == 2) {
                bool status;
                if (getStatus(&status)) {
                    printf("  Status: %s\n", status ? "Enabled" : "Disabled");
                } else {
                    printf("  Error Occured\n");
                }
                valid_command = true;
            } else if ((strcmp(argv[2], "set") == 0) && argc == 4) {
                if ((strcmp(argv[3], "enable") == 0)) {
                    if (setStatus(true)) {
                        printf("  SPI Module Enabled\n");
                    } else {
                        printf("  Error Occured\n");
                    }
                    valid_command = true;
                } else if ((strcmp(argv[3], "disable") == 0)) {
                    if (setStatus(false)) {
                        printf("  SPI Module Disabled\n");
                    } else {
                        printf("  Error Occured\n");
                    }
                    valid_command = true;
                }
            }
        } else if ((strcmp(argv[1], "send") == 0) && argc == 3) {
            uint32_t data = (uint32_t)strtol(argv[2], NULL, 0);
            if (sendData(data)) {
                printf("  Sent Data: 0x%08X\n", data);
            } else {
                printf("  Error Occured\n");
            }
            valid_command = true;
        } else if ((strcmp(argv[1], "read") == 0) && argc == 2) {
            uint32_t data = 0;
            if (readData(&data)) {
                printf("  Data: 0x%08X\n", data);
            } else {
                printf("  Error Occured\n");
            }
            valid_command = true;
        } else if ((strcmp(argv[1], "rx") == 0 || strcmp(argv[1], "tx") == 0) && argc == 3) {
            if ((strcmp(argv[2], "status") == 0)) {
                bool empty, full, ovr, success;
                if ((strcmp(argv[1], "rx") == 0)) {
                    success = getRxStatus(&empty, &full, &ovr);
                } else if ((strcmp(argv[1], "tx") == 0)) {
                    success = getTxStatus(&empty, &full, &ovr);
                }
                if (success) {
                    printf("  Status: \n    Empty: %s\n    Full: %s\n    OVR: %s\n", empty ? "TRUE" : "FALSE", full ? "TRUE" : "FALSE", ovr ? "TRUE" : "FALSE");
                } else {
                    printf("  Error Occured\n");
                }
                valid_command = true;
            } else if ((strcmp(argv[2], "count") == 0)) {
                bool success;
                uint8_t count;
                if ((strcmp(argv[1], "rx") == 0)) {
                    success = getRxCount(&count);
                } else if ((strcmp(argv[1], "tx") == 0)) {
                    success = getTxCount(&count);
                }
                if (success) {
                    printf("  Count: %d\n", count);
                } else {
                    printf("  Error Occured\n");
                }
                valid_command = true;
            } else if ((strcmp(argv[2], "clearov") == 0)) {
                bool success;
                if ((strcmp(argv[1], "rx") == 0)) {
                    success = clearRxOV();
                } else if ((strcmp(argv[1], "tx") == 0)) {
                    success = clearTxOV();
                }
                if (success) {
                    printf("  Overflow Cleared\n");
                } else {
                    printf("  Error Occured\n");
                }
                valid_command = true;
            } else if ((strcmp(argv[2], "reset") == 0)) {
                bool success;
                if ((strcmp(argv[1], "rx") == 0)) {
                    success = resetRx();
                } else if ((strcmp(argv[1], "tx") == 0)) {
                    success = resetTx();
                }
                if (success) {
                    printf("  Reset FIFO Completed\n");
                } else {
                    printf("  Error Occured\n");
                }
                valid_command = true;
            }
        } else if ((strcmp(argv[1], "wordsize") == 0)) {
            if (argc == 2) {
                uint8_t size = 0;
                bool success = getWordsize(&size);
                if (success) {
                    printf("  Wordsize: %d\n", size);
                } else {
                    printf("  Error Occured\n");
                }
                valid_command = true;
            } else if ((strcmp(argv[2], "set") == 0) && argc == 4) {
                uint8_t size = (uint8_t)strtol(argv[3], NULL, 0);
                if (setWordsize(size)) {
                    printf("  Set wordsize: %d\n", size);
                } else {
                    printf("  Error Occured\n");
                }
                valid_command = true;
            }
        } else if ((strcmp(argv[1], "device") == 0)) {
            if (argc == 2) {
                uint8_t dev = 0;
                bool success = getDevice(&dev);
                if (success) {
                    printf("  Device: %d\n", dev);
                } else {
                    printf("  Error Occured\n");
                }
                valid_command = true;
            } else if ((strcmp(argv[2], "set") == 0) && argc == 4) {
                uint8_t dev = (uint8_t)strtol(argv[3], NULL, 0);
                if (setDevice(dev)) {
                    printf("  Set device: %d\n", dev);
                } else {
                    printf("  Error Occured\n");
                }
                valid_command = true;
            }
        } else if ((strcmp(argv[1], "0") == 0) || (strcmp(argv[1], "1") == 0) || (strcmp(argv[1], "2") == 0) || (strcmp(argv[1], "3") == 0)) {
            uint8_t dev = (uint8_t)strtol(argv[1], NULL, 10);
            if (argc > 2) {
                if ((strcmp(argv[2], "csmode") == 0)) {
                    if (argc == 3) {
                        bool mode;
                        bool success = getCSModeForDevice(dev, &mode);
                        if (success) {
                            printf("  Mode: %s\n", mode ? "Auto" : "Manual");
                        } else {
                            printf("  Error Occured\n");
                        }
                        valid_command = true;
                    } else if ((strcmp(argv[3], "set") == 0) && argc == 5) {
                        if ((strcmp(argv[4], "auto") == 0)) {
                            if (setCSModeForDevice(dev, true)) {
                                printf("  Device %d, CS_AUTO set to Auto\n", dev);
                            } else {
                                printf("  Error Occured\n");
                            }
                            valid_command = true;
                        } else if ((strcmp(argv[4], "manual") == 0)) {
                            if (setCSModeForDevice(dev, false)) {
                                printf("  Device %d, CS_AUTO set to Manual\n", dev);
                            } else {
                                printf("  Error Occured\n");
                            }
                            valid_command = true;
                        }
                    }
                } else  if ((strcmp(argv[2], "cs") == 0)) {
                    if (argc == 3) {
                        bool enable;
                        bool success = getCSEnableForDevice(dev, &enable);
                        if (success) {
                            printf("  CS: %s\n", enable ? "Asserted" : "Deasserted");
                        } else {
                            printf("  Error Occured\n");
                        }
                        valid_command = true;
                    } else if ((strcmp(argv[3], "set") == 0) && argc == 5) {
                        if ((strcmp(argv[4], "assert") == 0)) {
                            if (setCSEnableForDevice(dev, true)) {
                                printf("  Device %d, CS is Asserted\n", dev);
                            } else {
                                printf("  Error Occured\n");
                            }
                            valid_command = true;
                        } else if ((strcmp(argv[4], "deassert") == 0)) {
                            if (setCSEnableForDevice(dev, false)) {
                                printf("  Device %d, CS is Deasserted\n", dev);
                            } else {
                                printf("  Error Occured\n");
                            }
                            valid_command = true;
                        }
                    }
                } else  if ((strcmp(argv[2], "mode") == 0)) {
                    if (argc == 3) {
                        bool spo, sph;
                        bool success = getSPIModeForDevice(dev, &spo, &sph);
                        if (success) {
                            printf("  Mode: %d:%d\n", spo ? 1 : 0, sph ? 1 : 0);
                        } else {
                            printf("  Error Occured\n");
                        }
                        valid_command = true;
                    } else if ((strcmp(argv[3], "set") == 0) && argc == 6) {
                        if ((strcmp(argv[4], "0") == 0) && (strcmp(argv[5], "0") == 0)) {
                            if (setSPIModeForDevice(dev, false, false)) {
                                printf("  Device %d, SPI Mode is 0:0\n", dev);
                            } else {
                                printf("  Error Occured\n");
                            }
                            valid_command = true;
                        } else if ((strcmp(argv[4], "0") == 0) && (strcmp(argv[5], "1") == 0)) {
                            if (setSPIModeForDevice(dev, false, true)) {
                                printf("  Device %d, SPI Mode is 0:1\n", dev);
                            } else {
                                printf("  Error Occured\n");
                            }
                            valid_command = true;
                        } else if ((strcmp(argv[4], "1") == 0) && (strcmp(argv[5], "0") == 0)) {
                            if (setSPIModeForDevice(dev, true, false)) {
                                printf("  Device %d, SPI Mode is 1:0\n", dev);
                            } else {
                                printf("  Error Occured\n");
                            }
                            valid_command = true;
                        } else if ((strcmp(argv[4], "1") == 0) && (strcmp(argv[5], "1") == 0)) {
                            if (setSPIModeForDevice(dev, true, true)) {
                                printf("  Device %d, SPI Mode is 1:1\n", dev);
                            } else {
                                printf("  Error Occured\n");
                            }
                            valid_command = true;
                        }
                    }
                }
            }
        } else if ((strcmp(argv[1], "brd") == 0)) {
            if (argc == 2) {
                double brd = 0;
                bool success = getBRD(&brd);
                if (success) {
                    printf("  BRD: %lfHz\n", brd);
                } else {
                    printf("  Error Occured\n");
                }
                valid_command = true;
            } else if ((strcmp(argv[2], "set") == 0) && argc == 4) {
                double brd = strtod(argv[3], NULL);
                double real_brd;
                if (setBRD(brd) && getBRD(&real_brd)) {
                    printf("  Set BRD: %lfHz\n", real_brd);
                } else {
                    printf("  Error Occured\n");
                }
                valid_command = true;
            }
        } else if ((strcmp(argv[1], "debug") == 0) && argc == 2) {
            uint16_t debug;
            bool success = getDebug(&debug);
            if (success) {
                printf("  Debug: %02X\n", debug);
            } else {
                printf("  Error Occured\n");
            }
            valid_command = true;
        }
    }
    if (!valid_command) {
        printf("  command not understood\n");
    }
}
