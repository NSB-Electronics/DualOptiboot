#include "printf.h"
#include <atmel_start.h>
#if defined( USB_SERIAL )
#include "usb_start.h"
#endif /* USB_SERIAL */

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
#if defined( USB_SERIAL )
        cdcdf_acm_write( (uint8_t *)print_buf, pb_ndx );
        delay_ms( 10 );
#endif /* USB_SERIAL */
        pb_ndx = 0;
    }
}
