#include "extFlash.h"
#include "sam_ba_monitor.h"
#include <sam.h>
#include <stdlib.h>
#include <string.h>

// Manufacturer ID
#define ADESTO 0x1F
#define MACRONIX 0xC2
#define WINBOND 0xEF

// Device ID
#define AT25XE011 0x4200
#define AT25DF041B 0x4402
#define MX25R8035F 0x2814
#define MX25R6435F 0x2817
#define W25Q32FV 0x4016

uint32_t _memSize = 0;
uint32_t _prgmSpace = 0;
uint16_t _deviceId;
uint8_t  _imageFlashed = 0;
uint16_t _eraseSize = 256;

#define GPIO_PIN_FUNCTION_A 0
#define GPIO_PIN_FUNCTION_B 1
#define GPIO_PIN_FUNCTION_C 2
#define GPIO_PIN_FUNCTION_D 3
#define GPIO_PIN_FUNCTION_E 4
#define GPIO_PIN_FUNCTION_F 5
#define GPIO_PIN_FUNCTION_G 6
#define GPIO_PIN_FUNCTION_H 7
#define GPIO_PORTA 0
#define GPIO_PORTB 1

#define GPIO_PIN( n ) ( ( (n)&0x1Fu ) << 0 )
#define GPIO_PORT( n ) ( ( n ) >> 5 )
#define GPIO( port, pin ) ( ( ( (port)&0x7u ) << 5 ) + ( (pin)&0x1Fu ) )
#define GPIO_PIN_FUNCTION_OFF 0xffffffff

#define PA10 GPIO( GPIO_PORTA, 10 )
#define PA11 GPIO( GPIO_PORTA, 11 )
#define FLASH_CS GPIO( GPIO_PORTA, 13 )
#define PA24 GPIO( GPIO_PORTA, 24 )
#define PA25 GPIO( GPIO_PORTA, 25 )
#define USB_HOST_EN GPIO( GPIO_PORTA, 28 )
#define PB03 GPIO( GPIO_PORTB, 3 )
#define PB22 GPIO( GPIO_PORTB, 22 )
#define PB23 GPIO( GPIO_PORTB, 23 )

Port *p = PORT;

#define FLASH_SELECT p->Group[GPIO_PORTA].OUTCLR.reg = 1 << FLASH_CS
#define FLASH_UNSELECT p->Group[GPIO_PORTA].OUTSET.reg = 1 << FLASH_CS

void _helper_set_pin( uint32_t gpio, uint32_t function )
{
    uint8_t port = GPIO_PORT( gpio );
    uint8_t pin = GPIO_PIN( gpio );

    if( function == GPIO_PIN_FUNCTION_OFF ) {
        uint8_t tmp;
        tmp = PORT->Group[port].PINCFG[pin].reg;
        tmp &= ~PORT_PINCFG_PMUXEN;
        tmp |= 0 << PORT_PINCFG_PMUXEN_Pos;
        PORT->Group[port].PINCFG[pin].reg = tmp;
    }
    else {
        uint8_t tmp;
        tmp = p->Group[port].PINCFG[pin].reg;
        tmp &= ~PORT_PINCFG_PMUXEN;
        tmp |= 1 << PORT_PINCFG_PMUXEN_Pos;
        p->Group[port].PINCFG[pin].reg = tmp;

        if( pin & 1 ) {
            // Odd numbered pin
            uint8_t tmp;
            tmp = p->Group[port].PMUX[pin >> 1].reg;
            tmp &= ~PORT_PMUX_PMUXO_Msk;
            tmp |= PORT_PMUX_PMUXO( function & 0xffff );
            p->Group[port].PMUX[pin >> 1].reg = tmp;
        }
        else {
            // Even numbered pin
            uint8_t tmp;
            tmp = p->Group[port].PMUX[pin >> 1].reg;
            tmp &= ~PORT_PMUX_PMUXE_Msk;
            tmp |= PORT_PMUX_PMUXE( function & 0xffff );
            p->Group[port].PMUX[pin >> 1].reg = tmp;
        }
    }
}

void SPI_init()
{
    // Init the pins
    _helper_set_pin( PB03, PINMUX_PB03D_SERCOM5_PAD1 );
    _helper_set_pin( PB22, PINMUX_PB22D_SERCOM5_PAD2 );
    _helper_set_pin( PB23, PINMUX_PB23D_SERCOM5_PAD3 );
    PORT->Group[GPIO_PORTA].DIRSET.reg = 1 << FLASH_CS;
    FLASH_UNSELECT;

    // Enable the APBC bus
    PM->APBCMASK.reg |= PM_APBCMASK_SERCOM5;
    // GCLK
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( GCLK_CLKCTRL_ID_SERCOM5_CORE_Val ) |
                        GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN;
    while( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
        ;

    // Reset the module
    SERCOM5->SPI.CTRLA.bit.SWRST = 1;
    while( SERCOM5->SPI.SYNCBUSY.bit.SWRST )
        ;

    SERCOM5->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_MODE_SPI_MASTER | SERCOM_SPI_CTRLA_DOPO( 0x01 ) | SERCOM_SPI_CTRLA_DIPO( 0x01 );

    // Set SERCOM5 in SPI master mode
    SERCOM5->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_RXEN;
    while( SERCOM5->SPI.SYNCBUSY.bit.CTRLB )
        ;

    // Set the baud rate
    SERCOM5->SPI.BAUD.reg = 12;

    // Enable
    SERCOM5->SPI.CTRLA.bit.ENABLE = 1;
    while( SERCOM5->SPI.SYNCBUSY.bit.ENABLE )
        ;
}

uint8_t SPI_transfer( uint8_t b )
{
    SERCOM5->SPI.DATA.bit.DATA = b;
    while( SERCOM5->SPI.INTFLAG.bit.RXC == 0 )
        ;
    return SERCOM5->SPI.DATA.bit.DATA;
}

uint16_t FLASH_init()
{
    // Get manufacturer ID and JEDEC ID
    FLASH_SELECT;
    SPI_transfer( SPIFLASH_JEDECID );
    uint8_t manufacturerId = SPI_transfer( 0 );
    _deviceId = SPI_transfer( 0 );
    _deviceId <<= 8;
    _deviceId |= SPI_transfer( 0 );
    FLASH_UNSELECT;

    // Check against manufacturer ID and JEDEC ID
    switch( manufacturerId ) {
        case ADESTO:
            if( _deviceId == AT25XE011 ) {
                _memSize = 0x20000;
                _eraseSize = 256;
            }
            else if( _deviceId == AT25DF041B ) {
                _memSize = 0x80000;
                _eraseSize = 256;
            }
            break;
        case MACRONIX:
            if( _deviceId == MX25R8035F ) {
                _memSize = 0x100000;
                _eraseSize = 4096;
            }
            else if( _deviceId == MX25R6435F ) {
                _memSize = 0x800000;
                _eraseSize = 4096;
            }
            break;
        case WINBOND:
            if( _deviceId == W25Q32FV ) {
                _memSize = 0x40000;
                _eraseSize = 4096;
            }
            break;
        default:
            // Unknown manufacturer
            return 0;
    }

    return _eraseSize;
}

uint8_t FLASH_busy()
{
    FLASH_SELECT;
    SPI_transfer( SPIFLASH_STATUSREAD );
    uint8_t status = SPI_transfer( 0 );
    FLASH_UNSELECT;
    return ( status & 0x01 );
}

void FLASH_command( uint8_t cmd, uint8_t isWrite )
{
    if( isWrite ) {
        FLASH_command( SPIFLASH_WRITEENABLE, 0 );
        FLASH_UNSELECT;
    }

    while( FLASH_busy() )
        ;

    FLASH_SELECT;
    SPI_transfer( cmd );
}

uint8_t FLASH_readByte( uint32_t addr )
{
    FLASH_command( SPIFLASH_ARRAYREADLOWFREQ, 0 );
    SPI_transfer( addr >> 16 );
    SPI_transfer( addr >> 8 );
    SPI_transfer( addr );
    uint8_t result = SPI_transfer( 0 );
    FLASH_UNSELECT;
    return result;
}

void FLASH_writeBytes( uint32_t addr, uint8_t *data, uint16_t len )
{
    if( addr % _eraseSize == 0 ) FLASH_erasePage( addr );
    FLASH_command( SPIFLASH_BYTEPAGEPROGRAM, 1 );
    SPI_transfer( addr >> 16 );
    SPI_transfer( addr >> 8 );
    SPI_transfer( addr );
    for( ; len > 0; len-- ) {
        SPI_transfer( *data );
        data++;
    }
    FLASH_UNSELECT;
}

void FLASH_erasePage( uint32_t addr )
{
    if( _eraseSize == 256 ) {
        FLASH_command( SPIFLASH_PAGEERASE_256, 1 ); // WE not required
        if( _deviceId == AT25DF041B ) {
            // Page address consists of 11 page address bits PA<10:0> of
            // 256Bytes each
            SPI_transfer( ( ( uint8_t )( addr >> 8 ) ) & 0x07 ); // byte 1
            SPI_transfer( (uint8_t)addr );                       // byte 2
            SPI_transfer( 0 );                                   // byte 3 (dummy)
            FLASH_UNSELECT;
        }
        else {
            // Page address consists of 9 page address bits PA<8:0> of 256Bytes
            // each
            SPI_transfer( ( ( uint8_t )( addr >> 8 ) ) & 0x01 ); // byte 1
            SPI_transfer( (uint8_t)addr );                       // byte 2
            SPI_transfer( 0 );                                   // byte 3 (dummy)
            FLASH_UNSELECT;
        }
    }
    else if( _eraseSize == 4096 ) {
        FLASH_command( SPIFLASH_BLOCKERASE_4K, 1 );
        SPI_transfer( addr >> 16 );
        SPI_transfer( addr >> 8 );
        SPI_transfer( addr );
        FLASH_UNSELECT;
    }
}

void checkFlashImage()
{
    FLASH_init();

    // Global unprotect
    FLASH_command( SPIFLASH_STATUSWRITE, 1 );
    SPI_transfer( 0 );
    FLASH_UNSELECT;

    /* Memory Layout
     ~~~ |0                   10                  20                  30 | ...
     ~~~ |0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1| ...
     ~~~ +---------------------------------------------------------------+
     ~~~ |F L X I M G|:|X X X X|:|                                       | ...
     ~~~ + - - - - - - - - - - - - - - - +-------------------------------+
     ~~~ | ID String |S|Len|S| Binary Image data                         | ...
     ~~~ +---------------------------------------------------------------+
     ~~~~~~ */
    // Check for an image
    if( FLASH_readByte( FLASH_IMAGE_OFFSET + 0 ) != 'F' )
        return;
    else if( FLASH_readByte( FLASH_IMAGE_OFFSET + 1 ) != 'L' )
        goto erase;
    else if( FLASH_readByte( FLASH_IMAGE_OFFSET + 2 ) != 'X' )
        goto erase;
    else if( FLASH_readByte( FLASH_IMAGE_OFFSET + 6 ) != ':' )
        goto erase;
    else if( FLASH_readByte( FLASH_IMAGE_OFFSET + 11 ) != ':' )
        goto erase;

    // Grab internal flash memory parameters
    _prgmSpace = ( 0x40000 - 0x4000 );

    // Grab the image size and validate
    uint32_t imagesize = ( FLASH_readByte( FLASH_IMAGE_OFFSET + 7 ) << 24 ) |
                         ( FLASH_readByte( FLASH_IMAGE_OFFSET + 8 ) << 16 ) |
                         ( FLASH_readByte( FLASH_IMAGE_OFFSET + 9 ) << 8 ) |
                         FLASH_readByte( FLASH_IMAGE_OFFSET + 10 );

    if( imagesize == 0 || imagesize > _memSize || imagesize > _prgmSpace )
        goto erase;

    // Variables for moving the image to internal program space
    uint32_t i;
    uint32_t prgmSpaceAddr = _prgmSpace;
    uint32_t pageSizes[] = {8, 16, 32, 64, 128, 256, 512, 1024};
    uint32_t PAGE_SIZE = pageSizes[NVMCTRL->PARAM.bit.PSZ];
    uint16_t cacheIndex = 0;
    uint8_t *cache = (uint8_t *)malloc( sizeof( uint8_t ) * PAGE_SIZE );

    // Copy the image to program space one page at a time
    for( i = 0; i < imagesize; i++ ) {
        cache[cacheIndex++] =
            FLASH_readByte( FLASH_IMAGE_OFFSET + i + _eraseSize );
        if( cacheIndex == PAGE_SIZE ) {
            eraseFlash( prgmSpaceAddr );
            writeFlash( prgmSpaceAddr, cache, PAGE_SIZE );
            prgmSpaceAddr += PAGE_SIZE;
            cacheIndex = 0;
        }
    }

    if( cacheIndex > 0 ) {
        eraseFlash( prgmSpaceAddr );
        writeFlash( prgmSpaceAddr, cache, cacheIndex );
    }

    free( cache );
    _imageFlashed = 1;

erase : {
    uint32_t flashAddr = FLASH_IMAGE_OFFSET;
    for( ; flashAddr <= ( FLASH_IMAGE_OFFSET + imagesize );
         flashAddr += 0x8000 ) {
        FLASH_command( SPIFLASH_BLOCKERASE_32K, 1 );
        SPI_transfer( flashAddr >> 16 );
        SPI_transfer( flashAddr >> 8 );
        SPI_transfer( flashAddr );
        FLASH_UNSELECT;
    }
}

    if( _imageFlashed ) jump_to_application();
}
