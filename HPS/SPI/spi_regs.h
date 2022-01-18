// SPI IP 
// SPI IP Library Registers
// CSE4356-SoC | Fall 2021 | Term Project
// Deborah Jahaj and Nathan Fusselman

//=============================================================================
// Hardware Target
//=============================================================================

// Target Platform: DE1-SoC Board
// Target uC:       -
// System Clock:    -

// Hardware configuration:
// SPI IP core connected to light-weight Avalon bus

//=============================================================================
// Device includes, defines, and assembler directives
//=============================================================================

#ifndef SPI_REGS_H_
#define SPI_REGS_H_

#define OFS_DATA             0
#define OFS_STATUS           1
#define OFS_CONTROL          2
#define OFS_BRD              3

#define SPAN_IN_BYTES 32

#endif

