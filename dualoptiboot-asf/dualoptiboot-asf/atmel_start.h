#ifndef ATMEL_START_H_INCLUDED
#define ATMEL_START_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "driver_init.h"
#if defined( USB_SERIAL )
#include "usb_start.h"
#endif /* USB_SERIAL */

/**
 * Initializes MCU, drivers and middleware in the project
 **/
void atmel_start_init( void );

#ifdef __cplusplus
}
#endif
#endif
