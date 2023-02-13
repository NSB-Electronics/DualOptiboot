#include <atmel_start.h>
#include <usb_start.h>

/************************************************************************
1. If an optiboot image is present, use that and attempt to go
2. If optiboot is not present, check if there is an application available, see function check_start_application
3. If check_start_application detects BOOT_DOUBLE_TAP or was reset by bossac then stay in the bootloader and enter the sam_ba_monitor indefinitely**
4. If check_start_application fails, then stay in bootloader and enter the sam_ba_monitor indefinitly**                                                                     
************************************************************************/

int main( void )
{
    /* Initializes MCU, drivers and middle ware */
    atmel_start_init();

    cdcd_acm_example();
}
