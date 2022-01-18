// QE IP Example, qe.c
// QE Shell Command
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

#include <stdlib.h>          // EXIT_ codes
#include <stdio.h>           // printf
#include <stdbool.h>         // true
#include "qe_ip.h"           // QE IP library

int main(int argc, char* argv[])
{
    if (!qeOpen())
    {
        printf("Error opening QE\n");
        return EXIT_FAILURE;
    }

    enableChannel(0);
    setPosition(0, 0);
    setVelocityTimebase(5000000); // 100 ms
    while(true)
    {
        printf("%20d, %20d\n", getPosition(0), getVelocity(0));
        usleep(100000);
    }
    return EXIT_SUCCESS;
}

