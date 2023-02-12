#include "jump.h"
#include <atmel_start.h>

void jump()
{
    // Pointer to the application space in memory
    void ( *application_code_entry )( void );

    // Reset the stack pointer to the application starting address, then inform
    // the interrupt vector table that it has a new location
    __set_MSP( *(uint32_t *)APP_START_ADDR );
    SCB->VTOR = ( (uint32_t)APP_START_ADDR & SCB_VTOR_TBLOFF_Msk );

    // Set the application entry pointer to the address, then jump!
    application_code_entry = ( void ( * )( void ) )(unsigned *)( *(unsigned *)( APP_START_ADDR + 4 ) );
    application_code_entry();
}

bool check_for_application()
{
    // Dont use this, we will use DOUBLE_TAP instead
	// Check the reset cause
    //uint8_t reset_cause = PM->RCAUSE.reg;

    // If the reset cause is either external (EXT), or system reset request (SYST) then we want to stay in the boot loader
    //if( reset_cause == PM_RCAUSE_EXT || reset_cause == PM_RCAUSE_SYST ) return false;

    // If there is something at the start address, assume it is a valid application.
    uint32_t *app_start = (uint32_t *)APP_START_ADDR;
    if( *app_start != INVALID_APP ) return true;

    // Otherwise assume nothing is there
    return false;
}
