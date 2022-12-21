#include "ext_flash.h"
#include "jump.h"
#include "printf.h"
#include "test_program.h"
#include <atmel_start.h>

#define OPTIBOOT_VERSION 1

void test_app()
{
    printf( "welcome to the test application, this loop will repeat\n" );

    int i = 0;
    while( 1 ) {
        delay_ms( 1000 );
        printf( "loop %d\n", i++ );
    }
}

int main( void )
{
    /* Initializes MCU, drivers and middleware */
    atmel_start_init();

    while( !cdcdf_acm_is_enabled() ) {
        // wait cdc acm to be installed
    };

    delay_ms( 10000 );

    //test_app();

    printf( "dualoptiboot version %d\n", OPTIBOOT_VERSION );

    SPI_init();
    // burn_image();
    check_flash_image();
    usbdc_detach();
    jump();
}
