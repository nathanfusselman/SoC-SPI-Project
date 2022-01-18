// QE IP Library Registers
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: DE1-SoC Board
// Target uC:       -
// System Clock:    -

// Hardware configuration:
// QE IP core connected to light-weight Avalon bus

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#ifndef QE_REGS_H_
#define QE_REGS_H_

#define OFS_CONTROL       0
#define OFS_PERIOD        1
#define OFS_POSITION0     4
#define OFS_VELOCITY0     5
#define OFS_POSITION1     6
#define OFS_VELOCITY1     7

#define QE_SPAN_IN_BYTES 32

#endif

