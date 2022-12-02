#include <extFlash.h>
#include <hardware.h>
#include <NVM.h>
#include <stdlib.h>
#include <string.h>
#include <optiboot.h>

// Manufacturer ID
#define ADESTO 0x1F
#define MACRONIX 0xC2

// Device ID
#define AT25XE011 0x4200
#define AT25DF041B 0x4402
#define MX25R8035F 0x2814
#define MX25R6435F 0x2817

#define FLASH_SELECT OUTPUT_LOW( FLASH_SS )
#define FLASH_UNSELECT OUTPUT_HIGH( FLASH_SS )

uint32_t _memSize = 0;
uint32_t _prgmSpace = 0;
uint16_t _deviceId;
uint8_t  _imageFlashed = 0;
uint16_t _eraseSize = 256;

void writeUUID( uint8_t *uuid )
{
    FLASH_writeBytes( 0, uuid, 8 );
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
            SPI_transfer( 0 ); // byte 3 (dummy)
            FLASH_UNSELECT;
        }
        else {
            // Page address consists of 9 page address bits PA<8:0> of 256Bytes
            // each
            SPI_transfer( ( ( uint8_t )( addr >> 8 ) ) & 0x01 ); // byte 1
            SPI_transfer( (uint8_t)addr );                       // byte 2
            SPI_transfer( 0 ); // byte 3 (dummy)
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
    NVMParams_t params = getNVMParams();
    _prgmSpace = ( params.nvmTotalSize - params.bootSize - params.eepromSize );

    // Grab the image size and validate
    uint32_t imagesize = ( FLASH_readByte( FLASH_IMAGE_OFFSET + 7 ) << 24 ) |
                         ( FLASH_readByte( FLASH_IMAGE_OFFSET + 8 ) << 16 ) |
                         ( FLASH_readByte( FLASH_IMAGE_OFFSET + 9 ) << 8 ) |
                         FLASH_readByte( FLASH_IMAGE_OFFSET + 10 );

    if( imagesize == 0 || imagesize > _memSize || imagesize > _prgmSpace )
        goto erase;

    // Variables for moving the image to internal program space
    uint32_t i;
#ifdef DEBUG
    uint32_t prgmSpaceAddr = 0x00008000;
#else
    uint32_t prgmSpaceAddr = params.bootSize;
#endif /* DEBUG */
    uint16_t cacheIndex = 0;
    uint8_t *cache = (uint8_t *)malloc( sizeof( uint8_t ) * params.rowSize );

    // Copy the image to program space one page at a time
    for( i = 0; i < imagesize; i++ ) {
        cache[cacheIndex++] =
            FLASH_readByte( FLASH_IMAGE_OFFSET + i + _eraseSize );
        if( cacheIndex == params.rowSize ) {
            eraseRow( prgmSpaceAddr );
            writeFlash( prgmSpaceAddr, cache, params.rowSize );
            prgmSpaceAddr += params.rowSize;
            cacheIndex = 0;
        }
    }

    if( cacheIndex > 0 ) {
        eraseRow( prgmSpaceAddr );
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

    if( _imageFlashed ) startApplication();
}