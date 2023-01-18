#ifndef SAM_BA_H_
#define SAM_BA_H_

#include "sam_ba.h"
#include <stdint.h>

#define SAM_BA_VERSION "2.0"

/* Selects USB as the communication interface of the monitor */
#define SAM_BA_INTERFACE_USBCDC 0
/* Selects USART as the communication interface of the monitor */
#define SAM_BA_INTERFACE_USART 1

/* Selects USB as the communication interface of the monitor */
#define SIZEBUFMAX 64

// Set this flag to let the bootloader enforce read restrictions of flash memory, even if security bit is not set
//#define SECURE_BY_DEFAULT

/**
 * \brief Initialize the monitor
 *
 */
void sam_ba_monitor_init( uint8_t com_interface );

/**
 * \brief Main function of the SAM-BA Monitor
 *
 */
void sam_ba_monitor_run( void );

/**
 * \brief
 *
 */
void sam_ba_putdata_term( uint8_t *data, uint32_t length );

/**
 * \brief
 *
 */
void call_applet( uint32_t address );

#endif /* SAM_BA_H_ */
