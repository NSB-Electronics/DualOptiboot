#include "ext_flash.h"
#include "jump.h"
#include "printf.h"
#include "sam_ba.h"
#include "test_program.h"
#include <atmel_start.h>

#define OPTIBOOT_VERSION 1

#if defined( BLD_TEST_APP )
void test_app()
{
    printf( "welcome to the test application, this loop will repeat\n" );

    int i = 0;
    while( 1 ) {
        delay_ms( 1000 );
        printf( "loop %d\n", i++ );
    }
}
#endif /* BLD_TEST_APP */

int main( void )
{
    /* Initializes MCU, drivers and middleware */
    atmel_start_init();

#if defined( USB_SERIAL )
    while( !cdcdf_acm_is_enabled() ) {
        // wait cdc acm to be installed
    };

    delay_ms( 10000 );
#endif /* USB_SERIAL */

#if defined( BLD_TEST_APP )
    // Define BLD_TEST_APP to create a sample program that can be utilized in the same
    // framework to test the bootloader.
    test_app();
#endif /* BLD_TEST_APP */

    printf( "dualoptiboot version %d\n", OPTIBOOT_VERSION );

    SPI_init();

#if defined( TEST_PRGM )
    // Define TEST_PRGM to burn a fixed image of the BLD_TEST_APP into SPI flash and back,
    // utilized for testing purposes only. WARNING - this will drastically increase the size
    // of the program and the user will have to account for this.
    burn_image();
#endif /* TEST_PRGM */

    check_flash_image();

#if defined( SAM_BA_USBCDC_ONLY )
#if !defined( USB_SERIAL )
    while( !cdcdf_acm_is_enabled() ) {
        // wait cdc acm to be installed
    };
#endif /* USB_SERIAL */
    sam_ba_monitor_init( SAM_BA_INTERFACE_USBCDC );
    sam_ba_monitor_run();
#endif /* SAM_BA_USBCDC_ONLY */

#if defined( USB_SERIAL )
    usbdc_detach();
#endif /* USB_SERIAL */

    jump();
}
