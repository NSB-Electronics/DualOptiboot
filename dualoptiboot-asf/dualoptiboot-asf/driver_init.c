/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include "driver_init.h"
#include <hal_init.h>
#include <hpl_gclk_base.h>
#include <hpl_pm_base.h>
#include <peripheral_clk_config.h>
#include <utils.h>

struct spi_m_sync_descriptor M0_SPI_FLASH;
struct spi_m_sync_descriptor RED_BOARD_SPI_FLASH;

struct flash_descriptor INTERNAL_FLASH;

void INTERNAL_FLASH_CLOCK_init( void )
{

    _pm_enable_bus_clock( PM_BUS_APBB, NVMCTRL );
}

void INTERNAL_FLASH_init( void )
{
    INTERNAL_FLASH_CLOCK_init();
    flash_init( &INTERNAL_FLASH, NVMCTRL );
}

void M0_SPI_FLASH_PORT_init( void )
{

    // Set pin direction to input
    gpio_set_pin_direction( PA12, GPIO_DIRECTION_IN );

    gpio_set_pin_pull_mode( PA12,
                            // <y> Pull configuration
                            // <id> pad_pull_config
                            // <GPIO_PULL_OFF"> Off
                            // <GPIO_PULL_UP"> Pull-up
                            // <GPIO_PULL_DOWN"> Pull-down
                            GPIO_PULL_OFF );

    gpio_set_pin_function( PA12, PINMUX_PA12D_SERCOM4_PAD0 );

    gpio_set_pin_level( PB10,
                        // <y> Initial level
                        // <id> pad_initial_level
                        // <false"> Low
                        // <true"> High
                        false );

    // Set pin direction to output
    gpio_set_pin_direction( PB10, GPIO_DIRECTION_OUT );

    gpio_set_pin_function( PB10, PINMUX_PB10D_SERCOM4_PAD2 );

    gpio_set_pin_level( PB11,
                        // <y> Initial level
                        // <id> pad_initial_level
                        // <false"> Low
                        // <true"> High
                        false );

    // Set pin direction to output
    gpio_set_pin_direction( PB11, GPIO_DIRECTION_OUT );

    gpio_set_pin_function( PB11, PINMUX_PB11D_SERCOM4_PAD3 );
}

void M0_SPI_FLASH_CLOCK_init( void )
{
    _pm_enable_bus_clock( PM_BUS_APBC, SERCOM4 );
    _gclk_enable_channel( SERCOM4_GCLK_ID_CORE, CONF_GCLK_SERCOM4_CORE_SRC );
}

void M0_SPI_FLASH_init( void )
{
    M0_SPI_FLASH_CLOCK_init();
    spi_m_sync_init( &M0_SPI_FLASH, SERCOM4 );
    M0_SPI_FLASH_PORT_init();
}

void RED_BOARD_SPI_FLASH_PORT_init( void )
{

    // Set pin direction to input
    gpio_set_pin_direction( PB03, GPIO_DIRECTION_IN );

    gpio_set_pin_pull_mode( PB03,
                            // <y> Pull configuration
                            // <id> pad_pull_config
                            // <GPIO_PULL_OFF"> Off
                            // <GPIO_PULL_UP"> Pull-up
                            // <GPIO_PULL_DOWN"> Pull-down
                            GPIO_PULL_OFF );

    gpio_set_pin_function( PB03, PINMUX_PB03D_SERCOM5_PAD1 );

    gpio_set_pin_level( PB22,
                        // <y> Initial level
                        // <id> pad_initial_level
                        // <false"> Low
                        // <true"> High
                        false );

    // Set pin direction to output
    gpio_set_pin_direction( PB22, GPIO_DIRECTION_OUT );

    gpio_set_pin_function( PB22, PINMUX_PB22D_SERCOM5_PAD2 );

    gpio_set_pin_level( PB23,
                        // <y> Initial level
                        // <id> pad_initial_level
                        // <false"> Low
                        // <true"> High
                        false );

    // Set pin direction to output
    gpio_set_pin_direction( PB23, GPIO_DIRECTION_OUT );

    gpio_set_pin_function( PB23, PINMUX_PB23D_SERCOM5_PAD3 );
}

void RED_BOARD_SPI_FLASH_CLOCK_init( void )
{
    _pm_enable_bus_clock( PM_BUS_APBC, SERCOM5 );
    _gclk_enable_channel( SERCOM5_GCLK_ID_CORE, CONF_GCLK_SERCOM5_CORE_SRC );
}

void RED_BOARD_SPI_FLASH_init( void )
{
    RED_BOARD_SPI_FLASH_CLOCK_init();
    spi_m_sync_init( &RED_BOARD_SPI_FLASH, SERCOM5 );
    RED_BOARD_SPI_FLASH_PORT_init();
}

void USB_DEVICE_INSTANCE_PORT_init( void )
{

    gpio_set_pin_direction( PA24,
                            // <y> Pin direction
                            // <id> pad_direction
                            // <GPIO_DIRECTION_OFF"> Off
                            // <GPIO_DIRECTION_IN"> In
                            // <GPIO_DIRECTION_OUT"> Out
                            GPIO_DIRECTION_OUT );

    gpio_set_pin_level( PA24,
                        // <y> Initial level
                        // <id> pad_initial_level
                        // <false"> Low
                        // <true"> High
                        false );

    gpio_set_pin_pull_mode( PA24,
                            // <y> Pull configuration
                            // <id> pad_pull_config
                            // <GPIO_PULL_OFF"> Off
                            // <GPIO_PULL_UP"> Pull-up
                            // <GPIO_PULL_DOWN"> Pull-down
                            GPIO_PULL_OFF );

    gpio_set_pin_function( PA24,
                           // <y> Pin function
                           // <id> pad_function
                           // <i> Auto : use driver pinmux if signal is imported by driver, else turn off function
                           // <PINMUX_PA24G_USB_DM"> Auto
                           // <GPIO_PIN_FUNCTION_OFF"> Off
                           // <GPIO_PIN_FUNCTION_A"> A
                           // <GPIO_PIN_FUNCTION_B"> B
                           // <GPIO_PIN_FUNCTION_C"> C
                           // <GPIO_PIN_FUNCTION_D"> D
                           // <GPIO_PIN_FUNCTION_E"> E
                           // <GPIO_PIN_FUNCTION_F"> F
                           // <GPIO_PIN_FUNCTION_G"> G
                           // <GPIO_PIN_FUNCTION_H"> H
                           PINMUX_PA24G_USB_DM );

    gpio_set_pin_direction( PA25,
                            // <y> Pin direction
                            // <id> pad_direction
                            // <GPIO_DIRECTION_OFF"> Off
                            // <GPIO_DIRECTION_IN"> In
                            // <GPIO_DIRECTION_OUT"> Out
                            GPIO_DIRECTION_OUT );

    gpio_set_pin_level( PA25,
                        // <y> Initial level
                        // <id> pad_initial_level
                        // <false"> Low
                        // <true"> High
                        false );

    gpio_set_pin_pull_mode( PA25,
                            // <y> Pull configuration
                            // <id> pad_pull_config
                            // <GPIO_PULL_OFF"> Off
                            // <GPIO_PULL_UP"> Pull-up
                            // <GPIO_PULL_DOWN"> Pull-down
                            GPIO_PULL_OFF );

    gpio_set_pin_function( PA25,
                           // <y> Pin function
                           // <id> pad_function
                           // <i> Auto : use driver pinmux if signal is imported by driver, else turn off function
                           // <PINMUX_PA25G_USB_DP"> Auto
                           // <GPIO_PIN_FUNCTION_OFF"> Off
                           // <GPIO_PIN_FUNCTION_A"> A
                           // <GPIO_PIN_FUNCTION_B"> B
                           // <GPIO_PIN_FUNCTION_C"> C
                           // <GPIO_PIN_FUNCTION_D"> D
                           // <GPIO_PIN_FUNCTION_E"> E
                           // <GPIO_PIN_FUNCTION_F"> F
                           // <GPIO_PIN_FUNCTION_G"> G
                           // <GPIO_PIN_FUNCTION_H"> H
                           PINMUX_PA25G_USB_DP );
}

/* The USB module requires a GCLK_USB of 48 MHz ~ 0.25% clock
 * for low speed and full speed operation. */
#if( CONF_GCLK_USB_FREQUENCY > ( 48000000 + 48000000 / 400 ) ) || ( CONF_GCLK_USB_FREQUENCY < ( 48000000 - 48000000 / 400 ) )
#warning USB clock should be 48MHz ~ 0.25% clock, check your configuration!
#endif

void USB_DEVICE_INSTANCE_CLOCK_init( void )
{

    _pm_enable_bus_clock( PM_BUS_APBB, USB );
    _pm_enable_bus_clock( PM_BUS_AHB, USB );
    _gclk_enable_channel( USB_GCLK_ID, CONF_GCLK_USB_SRC );
}

void USB_DEVICE_INSTANCE_init( void )
{
    USB_DEVICE_INSTANCE_CLOCK_init();
    usb_d_init();
    USB_DEVICE_INSTANCE_PORT_init();
}

void system_init( void )
{
    init_mcu();

    // GPIO on PA04

    gpio_set_pin_level( M0_SPI_FLASH_CS,
                        // <y> Initial level
                        // <id> pad_initial_level
                        // <false"> Low
                        // <true"> High
                        true );

    // Set pin direction to output
    gpio_set_pin_direction( M0_SPI_FLASH_CS, GPIO_DIRECTION_OUT );

    gpio_set_pin_function( M0_SPI_FLASH_CS, GPIO_PIN_FUNCTION_OFF );

    // GPIO on PA13

    gpio_set_pin_level( RED_BOARD_SPI_FLASH_CS,
                        // <y> Initial level
                        // <id> pad_initial_level
                        // <false"> Low
                        // <true"> High
                        true );

    // Set pin direction to output
    gpio_set_pin_direction( RED_BOARD_SPI_FLASH_CS, GPIO_DIRECTION_OUT );

    gpio_set_pin_function( RED_BOARD_SPI_FLASH_CS, GPIO_PIN_FUNCTION_OFF );

    INTERNAL_FLASH_init();

    M0_SPI_FLASH_init();

    RED_BOARD_SPI_FLASH_init();

#if defined( USB_SERIAL )
    USB_DEVICE_INSTANCE_init();
#endif /* USB_SERIAL */
}
