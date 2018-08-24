#include <optiboot.h>
#include <hardware.h>
#include <NVM.h>
#include <extFlash.h>
#include <serial.h>
#include <string.h>

const char *imageKey = "arduinoEImageKey";

void setImageKey()
{
    eraseRow( IMAGE_PRESENT_ADDR );
    writeFlash( IMAGE_PRESENT_ADDR, imageKey, 16 );
}

int checkImageKey()
{
    char key[16];
    readFlash( IMAGE_PRESENT_ADDR, key, 16 );
    return memcmp( key, imageKey, 16 );
}

void startApplication()
{
    // Take down the current application
    cleanUp();

    // Pointer to the application space in memory
    void ( *application_code_entry )( void );

    // Reset the stack pointer to the application starting address, then inform
    // the interrupt vector table that it has a new location
    __set_MSP( *(uint32_t *)APP_START_ADDR );
    SCB->VTOR = ( (uint32_t)APP_START_ADDR & SCB_VTOR_TBLOFF_Msk );

    // Set the application entry pointer to the address, then jump!
    application_code_entry = ( void ( * )( void ) )(
        unsigned *)( *(unsigned *)( APP_START_ADDR + 4 ) );
    application_code_entry();
}

int main( void )
{
    initHardware();
    checkFlashImage();
    if( checkImageKey() == 0 ) startApplication();
    serialConsole();
}