// SPI GPIO Expander
// SPI GPIO Expander Registers
// CSE4356-SoC | Fall 2021 | Term Project
// Deborah Jahaj and Nathan Fusselman

//=============================================================================
// Hardware Target
//=============================================================================

// Target Platform: DE1-SoC Board
// Target uC:       -
// System Clock:    -

// Hardware configuration:
// GPIO IP core connected to light-weight Avalon bus

//=============================================================================
// Device includes, defines, and assembler directives
//=============================================================================

#ifndef GPIO_EXPANDER_REGS_H_
#define GPIO_EXPANDER_REGS_H_

#define READ                 0x410000
#define WRITE                0x400000

#define IODIR                0x000000
#define IPOL                 0x000100
#define GPINTEN              0x000200
#define DEFVAL               0x000300
#define INTCON               0x000400
#define IOCON                0x000500
#define GPPU                 0x000600
#define INTF                 0x000700
#define INTCAP               0x000800
#define GPIO                 0x000900
#define OLAT                 0x000A00

#define BLANK                0x0000FF

#endif

