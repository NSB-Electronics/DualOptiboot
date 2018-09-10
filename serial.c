#include <serial.h>
#include <extFlash.h>
#include <stdlib.h>
#include <string.h>

uint8_t serialBuff[SERIAL_BUFF_SIZE];
uint8_t serialIndex = 0;
uint8_t serialLength = 0;
uint8_t numSerialLengthBytes = 0;
uint8_t serialLengthisSet = 0;
char    ack[] = {'~', 'O', '?', 'P'};

uint8_t  flashComplete = 0;
uint8_t  flashBuff[SERIAL_BUFF_SIZE];
uint16_t  flashBuffIndex = 0;
uint16_t imageCRC = 0;
uint32_t flashAddr = 256;
uint32_t imageSize = 0;

void sendAck()
{
    UART_write( ack[0] );
    UART_write( ack[1] );
    UART_write( ack[2] );
    UART_write( ack[3] );
}

uint8_t makeHex( uint8_t ch )
{
    if( ch >= '0' && ch <= '9' )
        ch -= '0';
    else if( ch >= 'A' && ch <= 'F' )
        ch -= 55;
    else if( ch >= 'a' && ch <= 'f' )
        ch -= 87;
    return ch;
}

uint8_t getHex( uint8_t *buff )
{
    uint8_t byte = makeHex( *buff );
    byte <<= 4;
    byte |= makeHex( *( buff + 1 ) );
    return byte;
}

void writeToFlash( uint16_t len )
{
    if( ( flashAddr % 256 ) == 0 ) FLASH_erasePage( flashAddr );
    FLASH_writeBytes( flashAddr, flashBuff, len );
    flashAddr += len;
    imageSize += len;
}

void burnFlashHeader()
{
    uint8_t header[12] = {'F', 'L', 'X', 'I', 'M', 'G', ':'};

    header[7] = ( imageSize >> 24 ) & 0xFF;
    header[8] = ( imageSize >> 16 ) & 0xFF;
    header[9] = ( imageSize >> 8 ) & 0xFF;
    header[10] = imageSize & 0xFF;

    header[11] = ':';

    FLASH_erasePage( 0 );
    FLASH_writeBytes( 0, header, 12 );
}

void runBootLoader( uint8_t hex )
{
    flashBuff[flashBuffIndex++] = hex;
    if( flashBuffIndex == 256 ) {
        writeToFlash( flashBuffIndex );
        flashBuffIndex = 0;
    }
}

void serialFormatToAMRFormat()
{
    char c = serialBuff[numSerialLengthBytes + 2];
    if( serialBuff[numSerialLengthBytes + 1] == RF_WS_OTA_KEY ) {
        switch( c ) {
            case ':':
                for( uint8_t i = 0; i < ( serialLength - 3 ); i += 2 )
                    runBootLoader(
                        getHex( &serialBuff[numSerialLengthBytes + 3 + i] ) );

                sendAck();
                break;
            case '?':
                if( serialBuff[numSerialLengthBytes + 3] == 'I' ) {
                    char *crc =
                        (char *)malloc( sizeof( char ) * ( serialLength - 3 ) );
                    memcpy( crc, &serialBuff[numSerialLengthBytes + 4],
                            ( serialLength - 4 ) );
                    crc[serialLength - 4] = 0;

                    uint16_t crcInt = atoi( crc );
                    imageCRC = 0;
                    imageCRC |= crcInt & 0xFF;
                    imageCRC |= ( crcInt >> 8 ) & 0xFF;
                    free( crc );

                    sendAck();
                }
                else if( serialBuff[numSerialLengthBytes + 3] == 'E' ) {
                    writeToFlash( flashBuffIndex );
                    flashBuffIndex = 0;
                    flashComplete = 1;
                }
                break;
        }
    }
    else if( serialBuff[numSerialLengthBytes + 1] == RF_WS_CMD_KEY ) {
        if( serialBuff[numSerialLengthBytes + 2] == '7' )
            sendAck();
    }
}

void getSerialLength()
{
    // Find out how many length bytes there are
    for( uint8_t i = 0; i < serialIndex; i++ ) {
        if( serialBuff[i] != RF_WS_INBOUND_DELIMITER )
            numSerialLengthBytes++;
        else
            break;
    }

    // Copy the length bytes over to integer format
    char *str = (char *)malloc( sizeof( char ) * ( numSerialLengthBytes + 1 ) );
    memcpy( str, serialBuff, numSerialLengthBytes );
    str[numSerialLengthBytes] = 0;
    serialLength = atoi( str );
    free( str );

    serialLengthisSet = 1;
}

void serialParser( uint8_t c )
{
    if( serialIndex < SERIAL_BUFF_SIZE ) {
        serialBuff[serialIndex++] = c;

        // If we have enough bytes then we should get the length
        if( serialIndex > 1 && !serialLengthisSet ) getSerialLength();

        // Once we have the entire command, parse it and reset the state machine
        if( serialLengthisSet &&
            ( ( serialIndex - numSerialLengthBytes ) >= serialLength ) ) {
            serialFormatToAMRFormat();
            serialIndex = 0;
            numSerialLengthBytes = 0;
            serialLengthisSet = 0;
        }
    }
    else {
        serialIndex = 0;
        serialLengthisSet = 0;
    }
}

void serialConsole()
{
    while( 1 ) {
		resetSerialTimeOut();
        uint8_t ch = UART_read();
		if( checkSerialTimeOut() )
			return;
		
        serialParser( ch );
        if( flashComplete ) {
            burnFlashHeader();
            checkFlashImage();
        }
    }
}
