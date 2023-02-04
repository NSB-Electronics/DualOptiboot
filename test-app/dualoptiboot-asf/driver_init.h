/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */
#ifndef DRIVER_INIT_INCLUDED
#define DRIVER_INIT_INCLUDED

#include "atmel_start_pins.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <hal_atomic.h>
#include <hal_delay.h>
#include <hal_gpio.h>
#include <hal_init.h>
#include <hal_io.h>
#include <hal_sleep.h>

#include <hal_flash.h>

#include <hal_spi_m_sync.h>
#include <hal_spi_m_sync.h>

#include "hal_usb_device.h"

extern struct flash_descriptor      INTERNAL_FLASH;
extern struct spi_m_sync_descriptor M0_SPI_FLASH;
extern struct spi_m_sync_descriptor RED_BOARD_SPI_FLASH;

void INTERNAL_FLASH_init(void);
void INTERNAL_FLASH_CLOCK_init(void);

void M0_SPI_FLASH_PORT_init(void);
void M0_SPI_FLASH_CLOCK_init(void);
void M0_SPI_FLASH_init(void);

void RED_BOARD_SPI_FLASH_PORT_init(void);
void RED_BOARD_SPI_FLASH_CLOCK_init(void);
void RED_BOARD_SPI_FLASH_init(void);

void USB_DEVICE_INSTANCE_CLOCK_init(void);
void USB_DEVICE_INSTANCE_init(void);

/**
 * \brief Perform system initialization, initialize pins and clocks for
 * peripherals
 */
void system_init(void);

#ifdef __cplusplus
}
#endif
#endif // DRIVER_INIT_INCLUDED
