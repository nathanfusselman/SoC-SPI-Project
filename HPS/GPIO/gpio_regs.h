// GPIO IP Library Registers
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: DE1-SoC Board
// Target uC:       -
// System Clock:    -

// Hardware configuration:
// GPIO IP core connected to light-weight Avalon bus

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#ifndef GPIO_REGS_H_
#define GPIO_REGS_H_

#define OFS_DATA             0
#define OFS_OUT              1
#define OFS_OD               2
#define OFS_INT_ENABLE       3
#define OFS_INT_POSITIVE     4
#define OFS_INT_NEGATIVE     5
#define OFS_INT_EDGE_MODE    6
#define OFS_INT_STATUS_CLEAR 7

#define SPAN_IN_BYTES 32

#define GPIO_IRQ 80

#endif

