// QE IP Example
// QE IP Library (qe_ip.c)
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: DE1-SoC Board

// Hardware configuration:
// QE 0 and 1:
//   GPIO_1[29-28] are used for QE 0 inputs B and A
//   GPIO_1[31-30] are used for QE 1 inputs B and A
// HPS interface:
//   Mapped to offset of 0x1000 in light-weight MM interface aperature

//-----------------------------------------------------------------------------

#include <stdint.h>          // C99 integer types -- uint32_t
#include <stdbool.h>         // bool
#include <fcntl.h>           // open
#include <sys/mman.h>        // mmap
#include <unistd.h>          // close
#include "address_map.h"     // address map
#include "qe_ip.h"           // qe
#include "qe_regs.h"         // registers

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

uint32_t *base = NULL;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

bool qeOpen()
{
    // Open /dev/mem
    int file = open("/dev/mem", O_RDWR | O_SYNC);
    bool bOK = (file >= 0);
    if (bOK)
    {
        // Create a map from the physical memory location of
        // /dev/mem at an offset to LW avalon interface
        // with an aperature of QE_SPAN_IN_BYTES bytes
        // to any location in the virtual 32-bit memory space of the process
        base = mmap(NULL, QE_SPAN_IN_BYTES, PROT_READ | PROT_WRITE, MAP_SHARED,
                    file, LW_BRIDGE_BASE + QE_BASE_OFFSET);
        bOK = (base != MAP_FAILED);

        // Close /dev/mem
        close(file);
    }
    return bOK;
}

void enableChannel(uint8_t channel)
{
    *(base+OFS_CONTROL) |= 1 << channel;
}

void disableChannel(uint8_t channel)
{
    *(base+OFS_CONTROL) &= ~(1 << channel);
}

void enableChannelSwap(uint8_t channel)
{
    *(base+OFS_CONTROL) |= 4 << channel;
}

void disableChannelSwap(uint8_t channel)
{
    *(base+OFS_CONTROL) &= ~(4 << channel);
}

void setPosition(uint8_t channel, int32_t position)
{
    *(base+OFS_POSITION0+channel*2) = position;
}

int32_t getPosition(uint8_t channel)
{
    return *(base+OFS_POSITION0+channel*2);
}

void setVelocityTimebase(uint32_t period)
{
    *(base+OFS_PERIOD) = period;
}

int32_t getVelocity(uint8_t channel)
{
    return *(base+OFS_VELOCITY0+channel*2);
}
