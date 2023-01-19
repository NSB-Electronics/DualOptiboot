#include "printf.h"
#include <atmel_start.h>

#define PB_LEN 1024
char print_buf[PB_LEN];
int  pb_ndx = 0;

void _block_interrupts()
{
    // Do nothing
}

void _unblock_interrupts()
{
    // Do nothing
}

void _putchar( char character )
{
    print_buf[pb_ndx++] = character;
    pb_ndx %= PB_LEN;
    if( character == '\n' ) {
        // TODO: print to the USB
        pb_ndx = 0;
    }
}
