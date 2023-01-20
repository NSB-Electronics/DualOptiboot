#include "board_definitions.h"
#include "ext_flash.h"
#include "jump.h"
#include "sam_ba_cdc.h"
#include "sam_ba_monitor.h"
#include "sam_ba_usb.h"
#include "test_program.h"
#include <atmel_start.h>

#define OPTIBOOT_VERSION 1

#if defined( BLD_TEST_APP )
void test_app()
{
    int i = 0;
    while( 1 ) {
        delay_ms( 1000 );
    }
}
#endif /* BLD_TEST_APP */

static volatile bool main_b_cdc_enable = false;

int main( void )
{
    /* Initializes MCU, drivers and middleware */
    atmel_start_init();

#if defined( BLD_TEST_APP )
    // Define BLD_TEST_APP to create a sample program that can be utilized in the same
    // framework to test the bootloader.
    test_app();
#endif /* BLD_TEST_APP */

    SPI_init();

#if defined( TEST_PRGM )
    // Define TEST_PRGM to burn a fixed image of the BLD_TEST_APP into SPI flash and back,
    // utilized for testing purposes only. WARNING - this will drastically increase the size
    // of the program and the user will have to account for this.
    burn_image();
#endif /* TEST_PRGM */

    check_flash_image();

    P_USB_CDC pCdc;

    board_init();
    __enable_irq();

    pCdc = usb_init();

    /* Start the sys tick (1 ms) */
    SysTick_Config( 1000 );

    /* Wait for a complete enum on usb or a '#' char on serial line */
    while( 1 ) {
#if defined( SAM_BA_USBCDC_ONLY ) || defined( SAM_BA_BOTH_INTERFACES )
        if( pCdc->IsConfigured( pCdc ) != 0 ) {
            main_b_cdc_enable = true;
        }

        /* Check if a USB enumeration has succeeded and if comm port has been opened */
        if( main_b_cdc_enable ) {
            sam_ba_monitor_init( SAM_BA_INTERFACE_USBCDC );
            /* SAM-BA on USB loop */
            while( 1 ) {
                sam_ba_monitor_run();
            }
        }
#endif
    }

    jump();
}

void SysTick_Handler( void )
{
}
