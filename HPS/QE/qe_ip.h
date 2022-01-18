// QE IP Example
// QE IP Library (qe_ip.h)
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

#ifndef QE_H_
#define QE_H_

#include <stdint.h>
#include <stdbool.h>

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

bool qeOpen();

void enableChannel(uint8_t channel);
void disableChannel(uint8_t channel);
void enableChannelSwap(uint8_t channel);
void disableChannelSwap(uint8_t channel);

void setPosition(uint8_t channel, int32_t position);
int32_t getPosition(uint8_t channel);

void setVelocityTimebase(uint32_t period);
int32_t getVelocity(uint8_t channel);

#endif
