/*
 * serial.c
 *
 * Created: 12/13/2022 3:27:36 PM
 *  Author: budge
 */
#include "printf.h"
#if defined( ASF_4 )
#include <atmel_start.h>
#endif /* ASF_4 */
#if defined( USB_SERIAL ) && defined( ASF_4 )
#include "usb_start.h"
#endif /* USB_SERIAL && ASF_4 */

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
#if defined( USB_SERIAL ) && defined( ASF_4 )
        cdcdf_acm_write( (uint8_t *)print_buf, pb_ndx );
        delay_ms( 10 );
#endif /* USB_SERIAL && ASF_4 */
        pb_ndx = 0;
    }
}
