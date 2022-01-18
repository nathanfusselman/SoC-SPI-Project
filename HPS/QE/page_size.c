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
//   Mapped to offset of 128 in light-weight MM interface aperature

//-----------------------------------------------------------------------------

#include <stdlib.h>          // EXIT_ codes
#include <stdio.h>           // printf
#include <unistd.h>

int main(int argc, char* argv[])
{
    printf("Page size = %ld\n", sysconf(_SC_PAGE_SIZE));
    return EXIT_SUCCESS;
}

