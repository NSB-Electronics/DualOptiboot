/*
 * jump.c
 *
 * Created: 12/14/2022 11:47:30 AM
 *  Author: budge
 */
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
