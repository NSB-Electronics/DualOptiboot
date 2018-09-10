#ifndef HARDWARE_H_
#define HARDWARE_H_

#include <stdint.h>
#include <sam.h>

#define OUTPUT_LOW( x )                         \
    {                                           \
        PORT->Group[0].OUTCLR.reg = ( 1 << x ); \
    }
#define OUTPUT_HIGH( x )                        \
    {                                           \
        PORT->Group[0].OUTSET.reg = ( 1 << x ); \
    }
#define SET_PIN_OUTMODE( x )                                \
    {                                                       \
        PORT->Group[0].DIRSET.reg = ( uint32_t )( 1 << x ); \
    }
#define SET_ODD_PIN_PERIPH( pin, periph )                                   \
    {                                                                       \
        PORT->Group[0].PMUX[( pin >> 1 )].reg |= PORT_PMUX_PMUXO( periph ); \
        PORT->Group[0].PINCFG[pin].reg = PORT_PINCFG_PMUXEN;                \
    }
#define SET_EVEN_PIN_PERIPH( pin, periph )                                  \
    {                                                                       \
        PORT->Group[0].PMUX[( pin >> 1 )].reg |= PORT_PMUX_PMUXE( periph ); \
        PORT->Group[0].PINCFG[pin].reg = PORT_PINCFG_PMUXEN;                \
    }

#ifndef FCPU
#define FCPU 8000000
#endif /* FCPU */

// GPIO Settings
#define RFM_SS 18   // PA18
#define FLASH_SS 23 // PA23
#define MOSI 16     // PA16
#define MISO 19     // PA19
#define SCK 17      // PA17
#define UART_RX 25  // PA25
#define UART_TX 24  // PA24

// SPI Settings
#define SPI_DATA_OUTPUT_PAD 0
#define SPI_DATA_INPUT_PAD 3
#define SPI_BAUD_RATE 1000000

// UART Settings
#define UART_TX_PAD 1
#define UART_RX_PAD 3
#define UART_BAUD_RATE 115200

void    initHardware();
uint8_t SPI_transfer( uint8_t _data );
void    UART_write( uint8_t _data );
uint8_t UART_read();
void    cleanUp();
uint8_t checkSerialTimeOut();
void    resetSerialTimeOut();

#endif /* HARDWARE_H_ */