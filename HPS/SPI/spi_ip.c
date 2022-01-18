// SPI IP
// SPI IP Library
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

#include <stdint.h>          // C99 integer types -- uint32_t
#include <stdbool.h>         // bool
#include <fcntl.h>           // open
#include <math.h>            // math
#include <sys/mman.h>        // mmap
#include <unistd.h>          // close
#include "../address_map.h"  // address map
#include "spi_ip.h"          // gpio
#include "spi_regs.h"        // registers
#include <stdio.h>

#define SYSTEM_CLOCK 50000000

//=============================================================================
// Global variables
//=============================================================================

uint32_t *base = NULL;

//=============================================================================
// Subroutines
//=============================================================================

bool spiOpen()
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
                    file, LW_BRIDGE_BASE + SPI_BASE_OFFSET);
        bOK = (base != MAP_FAILED);

        // Close /dev/mem
        close(file);
    }
    return bOK;
}

bool getStatus(bool *state)
{
    uint32_t control_reg = *(base+OFS_CONTROL);
    *state = control_reg & (1 << 15);
    return true;
}

bool setStatus(bool state)
{
    if (state) {
        *(base+OFS_CONTROL) |= (1 << 15);
    } else {
        *(base+OFS_CONTROL) &= ~(1 << 15);
    }
    bool setState;
    getStatus(&setState);
    return state == setState;
}

bool sendData(uint32_t data)
{
    bool empty, full, ovr;
    getTxStatus(&empty, &full, &ovr);
    if (full) return false;
    *(base+OFS_DATA) = data;
    usleep(10);
    return true;
}

bool readData(uint32_t *data)
{
    bool empty, full, ovr;
    getRxStatus(&empty, &full, &ovr);
    if (empty) return false;
    *data = *(base+OFS_DATA);
    usleep(10);
    return true;
}

bool getRxStatus(bool *empty, bool *full, bool *ovr)
{
    uint32_t status_reg = *(base+OFS_STATUS);
    *ovr = status_reg & ((1 << 0) << (3 * 0));
    *full = status_reg & ((1 << 1) << (3 * 0));
    *empty = status_reg & ((1 << 2) << (3 * 0));
    return true;
}

bool getTxStatus(bool *empty, bool *full, bool *ovr)
{
    uint32_t status_reg = *(base+OFS_STATUS);
    *ovr = status_reg & ((1 << 0) << (3 * 1));
    *full = status_reg & ((1 << 1) << (3 * 1));
    *empty = status_reg & ((1 << 2) << (3 * 1));
    return true;
}

bool getRxCount(uint8_t *count)
{
    uint32_t status_reg = *(base+OFS_STATUS);
    *count = (status_reg >> 8) & 0xF;
    return true;
}

bool getTxCount(uint8_t *count)
{
    uint32_t status_reg = *(base+OFS_STATUS);
    *count = (status_reg >> 12) & 0xF;
    return true;
}

bool clearRxOV()
{
    *(base+OFS_STATUS) = (1 << (3 * 0));
    bool empty, full, ovr;
    getRxStatus(&empty, &full, &ovr);
    return !ovr;
}

bool clearTxOV()
{
    *(base+OFS_STATUS) = (1 << (3 * 1));
    bool empty, full, ovr;
    getTxStatus(&empty, &full, &ovr);
    return !ovr;
}

bool resetRx()
{
    *(base+OFS_STATUS) = (1 << 6);
    return true;
}

bool resetTx()
{
    *(base+OFS_STATUS) = (1 << 7);
    return true;
}

bool getWordsize(uint8_t *size)
{
    uint32_t control_reg = *(base+OFS_CONTROL);
    *size = control_reg & 0x1F;
    *size = *size + 1;
    return true;
}

bool setWordsize(uint8_t size)
{
    if (size > 32) return false;
    *(base+OFS_CONTROL) &= ~0x1F;
    *(base+OFS_CONTROL) |= (size - 1) & 0x1F;
    return true;
}

bool getDevice(uint8_t *dev)
{
    if (*dev > 3) return false;
    uint32_t control_reg = *(base+OFS_CONTROL);
    *dev = (control_reg >> 13) & 0x3;
    return true;
}

bool setDevice(uint8_t dev)
{
    if (dev > 3) return false;
    *(base+OFS_CONTROL) &= ~(0x3 << 13);
    *(base+OFS_CONTROL) |= ((dev & 0x3) << 13);
    uint8_t newDev;
    getDevice(&newDev);
    return dev == newDev;
}

bool getCSModeForDevice(uint8_t dev, bool *mode)
{
    if (dev > 3) return false;
    uint32_t control_reg = *(base+OFS_CONTROL);
    *mode = (control_reg >> (5 + dev)) & 0x1;
    return true;
}

bool setCSModeForDevice(uint8_t dev, bool mode)
{
    if (dev > 3) return false;
    if (mode) {
        *(base+OFS_CONTROL) |= (1 << (5 + dev));
    } else {
        *(base+OFS_CONTROL) &= ~(1 << (5 + dev));
    }
    bool newMode;
    getCSModeForDevice(dev, &newMode);
    return mode == newMode;
}

bool getCSEnableForDevice(uint8_t dev, bool *enable)
{
    if (dev > 3) return false;
    uint32_t control_reg = *(base+OFS_CONTROL);
    *enable = (control_reg >> (9 + dev)) & 0x1;
    return true;
}

bool setCSEnableForDevice(uint8_t dev, bool enable)
{
    if (dev > 3) return false;
    if (enable) {
        *(base+OFS_CONTROL) |= (1 << (9 + dev));
    } else {
        *(base+OFS_CONTROL) &= ~(1 << (9 + dev));
    }
    bool newEnable;
    getCSEnableForDevice(dev, &newEnable);
    return enable == newEnable;
}

bool getSPIModeForDevice(uint8_t dev, bool *spo, bool *sph)
{
    if (dev > 3) return false;
    uint32_t control_reg = *(base+OFS_CONTROL);
    *spo = (control_reg >> (16 + (dev * 2))) & 0x1;
    *sph = (control_reg >> (17 + (dev * 2))) & 0x1;
    return true;
}

bool setSPIModeForDevice(uint8_t dev, bool spo, bool sph)
{
    if (dev > 3) return false;
    if (spo) {
        *(base+OFS_CONTROL) |= (1 << (16 + (dev * 2)));
    } else {
        *(base+OFS_CONTROL) &= ~(1 << (16 + (dev * 2)));
    }
    if (sph) {
        *(base+OFS_CONTROL) |= (1 << (17 + (dev * 2)));
    } else {
        *(base+OFS_CONTROL) &= ~(1 << (17 + (dev * 2)));
    }
    bool newSPO, newSPH;
    getSPIModeForDevice(dev, &newSPO, &newSPH);
    return spo == newSPO && sph == newSPH;
}

bool getBRD(double *brd)
{
    uint32_t raw_brd = *(base+OFS_BRD);
    double divisor = raw_brd >> 6;
    uint8_t i;
    for (i = 0; i < 6; i++) {
        if (raw_brd & (1 << (5 - i))) {
            divisor += pow(2,-i);
        }
    }
    *brd = ((double)SYSTEM_CLOCK / divisor);
    return true;
}

bool setBRD(double brd)
{
    double divisor = ((double)SYSTEM_CLOCK / brd);
    uint32_t brd_value = ((uint32_t)divisor) << 6;
    uint32_t integer_divisor = (uint32_t) divisor;
    uint8_t i;
    for (i = 0; i < 6; i++) {
        if (fmod((((double)integer_divisor)/ (0.5/(pow(2,i)))), 2) >= 1) {
            brd_value |= 1 << (5 - i);
        }
    }
    *(base+OFS_BRD) = brd_value;
    double check;
    getBRD(&check);
    return check > (brd - (brd*0.001)) && check < (brd + (brd*0.001));
}

bool getDebug(uint16_t *debug)
{
    uint32_t status_reg = *(base+OFS_STATUS);
    *debug = status_reg >> 16;
    return true;
}