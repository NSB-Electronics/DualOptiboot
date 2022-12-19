#include "ext_flash.h"
#include "atmel_start_pins.h"
#include "printf.h"
#include <atmel_start.h>
#include <stdbool.h>
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
uint16_t _deviceId = 0;
uint8_t  _imageFlashed = 0;
uint16_t _eraseSize = 256;
uint32_t _cs_pin = 0;

void select( bool sel )
{
    gpio_set_pin_level( _cs_pin, !sel );
}

void SPI_init()
{
#if defined( RED_BOARD_TURBO )
    spi_m_sync_enable( &RED_BOARD_SPI_FLASH );
    _cs_pin = RED_BOARD_SPI_FLASH_CS;
    printf( "configured for redboard turbo\n" );
#elif defined( ADAFRUIT_M0_FEATHER )
    spi_m_sync_enable( &M0_SPI_FLASH );
    _cs_pin = M0_SPI_FLASH_CS;
    printf( "configured for m0 feather\n" );
#else
    printf( "warning! unknown configuration\n" );
#endif
}

uint8_t SPI_transfer( uint8_t b )
{
    uint8_t tx = b;
    uint8_t rx = 0;
    //struct spi_m_sync_descriptor *spi = CONTAINER_OF( ext_flash_spi, struct spi_m_sync_descriptor, io );
    struct spi_xfer xfer;

    xfer.rxbuf = (uint8_t *)&rx;
    xfer.txbuf = (uint8_t *)&tx;
    xfer.size = 1;

#if defined( RED_BOARD_TURBO )
    spi_m_sync_transfer( &RED_BOARD_SPI_FLASH, &xfer );
#elif defined( ADAFRUIT_M0_FEATHER )
    spi_m_sync_transfer( &M0_SPI_FLASH, &xfer );
#endif

    return rx;
}

uint16_t FLASH_init()
{
    printf( "initializing spi flash...\n" );

    // Get manufacturer ID and JEDEC ID
    select( true );
    SPI_transfer( SPIFLASH_JEDECID );
    uint8_t manufacturerId = SPI_transfer( 0 );
    _deviceId = SPI_transfer( 0 );
    _deviceId <<= 8;
    _deviceId |= SPI_transfer( 0 );
    select( false );

    printf( "got spi flash device ID %X\n", _deviceId );

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
            else {
                _memSize = 0x40000;
                _eraseSize = 4096;
            }
            break;
        default:
            // Unknown manufacturer
            return 0;
    }

    printf( "configured for memory size %d, erase size %d\n", _memSize, _eraseSize );

    return _eraseSize;
}

uint8_t FLASH_busy()
{
    select( true );
    SPI_transfer( SPIFLASH_STATUSREAD );
    uint8_t status = SPI_transfer( 0 );
    select( false );
    return ( status & 0x01 );
}

void FLASH_command( uint8_t cmd, uint8_t isWrite )
{
    if( isWrite ) {
        FLASH_command( SPIFLASH_WRITEENABLE, 0 );
        select( false );
    }

    while( FLASH_busy() )
        ;

    select( true );
    SPI_transfer( cmd );
}

uint8_t FLASH_readByte( uint32_t addr )
{
    FLASH_command( SPIFLASH_ARRAYREADLOWFREQ, 0 );
    SPI_transfer( addr >> 16 );
    SPI_transfer( addr >> 8 );
    SPI_transfer( addr );
    uint8_t result = SPI_transfer( 0 );
    select( false );
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
    select( false );
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
            select( false );
        }
        else {
            // Page address consists of 9 page address bits PA<8:0> of 256Bytes
            // each
            SPI_transfer( ( ( uint8_t )( addr >> 8 ) ) & 0x01 ); // byte 1
            SPI_transfer( (uint8_t)addr );                       // byte 2
            SPI_transfer( 0 );                                   // byte 3 (dummy)
            select( false );
        }
    }
    else if( _eraseSize == 4096 ) {
        FLASH_command( SPIFLASH_BLOCKERASE_4K, 1 );
        SPI_transfer( addr >> 16 );
        SPI_transfer( addr >> 8 );
        SPI_transfer( addr );
        select( false );
    }
}

void check_flash_image()
{
    FLASH_init();

    // Global unprotect
    FLASH_command( SPIFLASH_STATUSWRITE, 1 );
    SPI_transfer( 0 );
    select( false );

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
    printf( "checking for FLXIMG header...\n" );
    if( FLASH_readByte( FLASH_IMAGE_OFFSET + 0 ) != 'F' ) {
        printf( "no header found\n" );
        return;
    }
    else if( FLASH_readByte( FLASH_IMAGE_OFFSET + 1 ) != 'L' )
        goto erase;
    else if( FLASH_readByte( FLASH_IMAGE_OFFSET + 2 ) != 'X' )
        goto erase;
    else if( FLASH_readByte( FLASH_IMAGE_OFFSET + 6 ) != ':' )
        goto erase;
    else if( FLASH_readByte( FLASH_IMAGE_OFFSET + 11 ) != ':' )
        goto erase;

    // Grab internal flash memory parameters
    _prgmSpace = ( 0x40000 ); // TODO: check on final size here

    // Grab the image size and validate
    uint32_t imagesize = ( FLASH_readByte( FLASH_IMAGE_OFFSET + 7 ) << 24 ) |
                         ( FLASH_readByte( FLASH_IMAGE_OFFSET + 8 ) << 16 ) |
                         ( FLASH_readByte( FLASH_IMAGE_OFFSET + 9 ) << 8 ) |
                         FLASH_readByte( FLASH_IMAGE_OFFSET + 10 );

    printf( "got valid header, image size is %d bytes\n", imagesize );
    if( imagesize == 0 || imagesize > _memSize || imagesize > _prgmSpace ) {
        imagesize = _prgmSpace;
        goto erase;
    }

    // Variables for moving the image to internal program space
    uint32_t prgmSpaceAddr = 0x8000; // TODO: don't use hard coded start address
    uint32_t PAGE_SIZE = flash_get_page_size( &INTERNAL_FLASH );
    uint16_t cacheIndex = 0;
    //uint8_t *cache = (uint8_t *)malloc( sizeof( uint8_t ) * PAGE_SIZE );
    uint8_t cache[1024];

    // Copy the image to program space one page at a time
    printf( "writing to program memory...\n" );
    for( uint32_t i = 0; i < imagesize; i++ ) {
        cache[cacheIndex++] = FLASH_readByte( FLASH_IMAGE_OFFSET + i + 12 );
        if( cacheIndex == PAGE_SIZE ) {
            printf( "erasing page addr %X\n", prgmSpaceAddr );
            flash_erase( &INTERNAL_FLASH, prgmSpaceAddr, 1 );
            printf( "Writing: " );
#if defined( DEBUG )
            for( int i = 0; i < cacheIndex; i++ ) printf( "%X", cache[i] );
#endif
            printf( " to memory at address %X\n", prgmSpaceAddr );
            flash_write( &INTERNAL_FLASH, prgmSpaceAddr, cache, PAGE_SIZE );
            prgmSpaceAddr += PAGE_SIZE;
            cacheIndex = 0;
        }
    }

    if( cacheIndex > 0 ) {
        printf( "erasing page addr %X\n", prgmSpaceAddr );
        flash_erase( &INTERNAL_FLASH, prgmSpaceAddr, 1 );
        printf( "Writing: " );
#if defined( DEBUG )
        for( int i = 0; i < cacheIndex; i++ ) printf( "%X", cache[i] );
#endif
        printf( " to memory at address %X\n", prgmSpaceAddr );
        flash_write( &INTERNAL_FLASH, prgmSpaceAddr, cache, PAGE_SIZE );
    }

    //free( cache );
    _imageFlashed = 1;

erase : {
    uint32_t flashAddr = FLASH_IMAGE_OFFSET;
    for( ; flashAddr <= ( FLASH_IMAGE_OFFSET + imagesize );
         flashAddr += 0x8000 ) {
        FLASH_command( SPIFLASH_BLOCKERASE_32K, 1 );
        SPI_transfer( flashAddr >> 16 );
        SPI_transfer( flashAddr >> 8 );
        SPI_transfer( flashAddr );
        select( false );
    }
}

    if( _imageFlashed ) {
        printf( "imaged flashed successfully, jumping to program now!\n" );
    }
    else {
        printf( "no image found\n" );
    }
}