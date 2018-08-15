#include <hardware.h>
#include <extFlash.h>
#include <serial.h>

int main( void )
{
    initHardware();
    CheckFlashImage();
    runSerialBootLoader();
}