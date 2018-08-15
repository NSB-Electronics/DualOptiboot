#include <serial.h>
#include <extFlash.h>
#include <stdlib.h>

#define SEND_ACK UART_write( '1' )
#define SEND_NACK UART_write( '0' )

uint8_t  ping[256];
uint8_t  pong[256];
uint8_t  usePong = 0;
uint8_t  writeTriggered = 0;
uint16_t ndx = 0;
uint32_t flashAddr = 256;
uint32_t imageSize = 0;
uint16_t codeSeg = 0;
uint16_t prgmCntr = 0;

uint8_t makeHex()
{
    uint8_t ch = UART_read();
    if( ch >= '0' && ch <= '9' )
        ch -= '0';
    else if( ch >= 'A' && ch <= 'F' )
        ch -= 55;
    else if( ch >= 'a' && ch <= 'f' )
        ch -= 87;
    return ch;
}

uint8_t getHex()
{
    uint8_t byte = makeHex();
    byte <<= 4;
    byte |= makeHex();
    return byte;
}

void writeToFlash( uint16_t len )
{
    if( ( flashAddr % 256 ) == 0 ) FLASH_erasePage( flashAddr );

    if( usePong )
        FLASH_writeBytes( flashAddr, ping, len );
    else
        FLASH_writeBytes( flashAddr, pong, len );

    flashAddr += len;
    imageSize += len;
}

void burnFlashHeader()
{
    uint8_t header[16] = {'F', 'L', 'X', 'I', 'M', 'G', ':'};

    header[7] = ( imageSize >> 24 ) & 0xFF;
    header[8] = ( imageSize >> 16 ) & 0xFF;
    header[9] = ( imageSize >> 8 ) & 0xFF;
    header[10] = imageSize & 0xFF;

    header[11] = ':';

    header[12] = ( codeSeg >> 8 ) & 0xFF;
    header[13] = codeSeg & 0xFF;

    header[14] = ( prgmCntr >> 8 ) & 0xFF;
    header[15] = prgmCntr & 0xFF;

    FLASH_erasePage( 0 );
    FLASH_writeBytes( 0, header, 16 );
}

void runSerialBootLoader()
{
    uint8_t type, len;

    flashAddr = 256;
    imageSize = 0;

    // TODO: WDT
    while( 1 ) {
        type = getHex();
        SEND_ACK;
        len = getHex();
        SEND_ACK;

        if( type == 1 ) {
            usePong ^= 1;
            writeToFlash( ndx );
            burnFlashHeader();
            break;
        }
        else if( type == 0 ) {
            for( ; len > 0; len-- ) {
                // Check for a full buffer
                if( ndx == 256 ) {
                    ndx = 0;
                    usePong ^= 1;
                    writeTriggered = 1;
                }

                // Fill the ping or pong buffer
                if( usePong )
                    pong[ndx++] = getHex();
                else
                    ping[ndx++] = getHex();
            }

            // Write to flash memory
            if( writeTriggered ) {
                writeTriggered = 0;
                writeToFlash( 256 );
            }

            SEND_ACK;
        }
        else if( type == 3 ) {
            uint8_t tmp[8];
            uint8_t i;

            // Record type 3 has the code segment and program counter to load
            for( i = 0; i < 4; i++ ) tmp[i] = getHex();
            memcpy( &codeSeg, tmp, sizeof( uint16_t ) );
            memcpy( &prgmCntr, &tmp[2], sizeof( uint16_t ) );

            SEND_ACK;
        }
    }
}