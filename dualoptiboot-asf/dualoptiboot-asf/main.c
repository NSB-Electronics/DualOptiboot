#include "board_definitions.h"
#include "ext_flash.h"
#include "jump.h"
#include "sam_ba_cdc.h"
#include "sam_ba_monitor.h"
#include "sam_ba_usb.h"
#include <atmel_start.h>

/************************************************************************
1. If an optiboot image is present, use that and attempt to go
2. If optiboot is not present, check if there is an application available, see function check_start_application
3. If check_start_application detects BOOT_DOUBLE_TAP or was reset by bossac then stay in the bootloader and enter the sam_ba_monitor indefinitely**
4. If check_start_application fails, then stay in bootloader and enter the sam_ba_monitor indefinitly**                                                                     
************************************************************************/

#define OPTIBOOT_VERSION 1

static volatile bool main_b_cdc_enable = false;
uint32_t timeout_cnt = 0;

bool check_for_double_tap()
{
#if defined(BOOT_DOUBLE_TAP_ADDRESS)
	
	#define DOUBLE_TAP_MAGIC 0x07738135
	if (PM->RCAUSE.reg & PM_RCAUSE_POR)
	{
		/* On power-on initialize double-tap */
		BOOT_DOUBLE_TAP_DATA = 0;
	}
	else
	{
		if (BOOT_DOUBLE_TAP_DATA == DOUBLE_TAP_MAGIC)
		{
			/* Second tap, stay in bootloader */
			BOOT_DOUBLE_TAP_DATA = 0;
			return true;
		}
		/* First tap */
		BOOT_DOUBLE_TAP_DATA = DOUBLE_TAP_MAGIC;

		/* Wait 0.5sec to see if the user tap reset again.
		 * The loop value is based on SAMD21 default 1MHz clock @ reset.
		 */
		for (uint32_t i=0; i<125000; i++) /* 500ms */
		  /* force compiler to not optimize this... */
		  __asm__ __volatile__("");

		/* Timeout happened, continue boot... */
		BOOT_DOUBLE_TAP_DATA = 0;
	 }
#endif

    // Otherwise no double tap detected
    return false;
}

int main( void )
{
	/* Check for double tap, must be before Initializes MCU as it clears the sram where double_Tap_magic is stored? */
	if (!check_for_double_tap()) {
		/* Initializes MCU, drivers and middle ware */
		atmel_start_init();
		SPI_init();
		
		// 1. Check for an optiboot image, jump if it's there.
		if( check_flash_image() ) jump();
		
		// 2. Check for an application, if we find one, try to jump.
		if( check_for_application() ) jump();
	} else {
		/* Skip checking flash or application and stay in bootloader */
	}
	
#if defined( SAM_BA_USBCDC_ONLY )
    P_USB_CDC pCdc;
	
    board_init();
    __enable_irq();

    pCdc = usb_init();

    SysTick_Config( 1000 );
	
    while( 1 ) {
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
		// Add timeout so we dont get stuck in the bootloader forever
		// Only runs when no USB is connected?
		timeout_cnt += 1;
		if (timeout_cnt > 1250000) {
			// 5 seconds have passed, reset
			NVIC_SystemReset();      // processor software reset
		}
    }
#endif /* SAM_BA_USBCDC_ONLY */
}

void SysTick_Handler( void )
{
}
