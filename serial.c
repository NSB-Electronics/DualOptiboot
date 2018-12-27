#include <serial.h>
#include <extFlash.h>
#include <stdlib.h>
#include <string.h>

#define ACK_LEN 6
#define SERIAL_OFFSET 1

uint8_t _hexBuff[2];
uint8_t _hexIndex = 0;

uint8_t _serialBuff[SERIAL_BUFF_SIZE];
uint8_t _serialIndex = 0;
uint8_t _serialLength = 0;
uint8_t _serialLengthisSet = 0;
uint8_t _ack[] = {'`', '~', 'O', '?', 'P', 0x00, '@'};

uint8_t  _flashComplete = 0;
uint8_t  _flashBuff[SERIAL_BUFF_SIZE];
uint16_t _flashBuffIndex = 0;
uint16_t _imageCRC = 0;
uint32_t _headAddr = 0;
uint32_t _imageSize = 0;

uint8_t hexify( uint8_t b )
{
    if( b >= 0 && b <= 9 )
        b += 0x30;
    else if( b >= 0xA && b <= 0xF )
        b += 55;
    return b;
}

void sendAck()
{
    uint8_t i = 1;
    UART_write( _ack[0] );
    for( ; i < ACK_LEN; i++ ) {
        UART_write( hexify( ( _ack[i] >> 4 ) & 0xF ) );
        UART_write( hexify( _ack[i] & 0xF ) );
    }
    UART_write( _ack[6] );
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
    FLASH_writeBytes( _headAddr, _flashBuff, len );
    _headAddr += len;
    _imageSize += len;
}

void burnFlashHeader()
{
    uint8_t header[12] = {'F', 'L', 'X', 'I', 'M', 'G', ':'};

    header[7] = ( _imageSize >> 24 ) & 0xFF;
    header[8] = ( _imageSize >> 16 ) & 0xFF;
    header[9] = ( _imageSize >> 8 ) & 0xFF;
    header[10] = _imageSize & 0xFF;

    header[11] = ':';

    FLASH_writeBytes( FLASH_IMAGE_OFFSET, header, 12 );
}

void runBootLoader( uint8_t hex )
{
    _flashBuff[_flashBuffIndex++] = hex;
    if( _flashBuffIndex == 256 ) {
        writeToFlash( _flashBuffIndex );
        _flashBuffIndex = 0;
    }
}

void serialFormatToAMRFormat()
{
    char c = _serialBuff[SERIAL_OFFSET + 2];
    if( _serialBuff[SERIAL_OFFSET + 1] == RF_WS_OTA_KEY ) {
        switch( c ) {
            case ':':
                for( uint8_t i = 0; i < ( _serialLength - 3 ); i++ )
                    runBootLoader( _serialBuff[SERIAL_OFFSET + 3 + i] );

                sendAck();
                break;
            case '?':
                if( _serialBuff[SERIAL_OFFSET + 3] == 'I' ) {
                    _imageCRC = _serialBuff[SERIAL_OFFSET + 4];
                    _imageCRC <<= 8;
                    _imageCRC |= _serialBuff[SERIAL_OFFSET + 5];

                    sendAck();
                }
                else if( _serialBuff[SERIAL_OFFSET + 3] == 'E' ) {
                    writeToFlash( _flashBuffIndex );
                    _flashBuffIndex = 0;
                    _flashComplete = 1;
                }
                break;
        }
    }
    else if( _serialBuff[SERIAL_OFFSET + 1] == RF_WS_CMD_KEY ) {
        if( _serialBuff[SERIAL_OFFSET + 2] == 0x7 ) sendAck();
    }
    else if( _serialBuff[SERIAL_OFFSET + 1] == RF_WS_BURN_UUID_KEY ) {
        uint8_t i;

        // Write the UUID to external Flash
        writeUUID( &_serialBuff[SERIAL_OFFSET + 2] );

        // Read it back as the ACK to the message & send it
        UART_write( _ack[0] );
        UART_write( hexify( ( _ack[1] >> 4 ) & 0xF ) );
        UART_write( hexify( _ack[1] & 0xF ) );
        UART_write( hexify( ( RF_WS_BURN_UUID_KEY >> 4 ) & 0xF ) );
        UART_write( hexify( RF_WS_BURN_UUID_KEY & 0xF ) );
        for( i = 0; i < 8; i++ ) {
            uint8_t byte = FLASH_readByte( i );
            UART_write( hexify( ( byte >> 4 ) & 0xF ) );
            UART_write( hexify( byte & 0xF ) );
        }
        UART_write( _ack[6] );
    }
}

void getSerialLength()
{
    _serialLength = _serialBuff[0] / 2;
    _serialLengthisSet = 1;
}

void serialParser( uint8_t c )
{
    if( _serialIndex < SERIAL_BUFF_SIZE ) {
        _serialBuff[_serialIndex++] = c;

        // If we have enough bytes then we should get the length
        if( _serialIndex > 1 && !_serialLengthisSet ) getSerialLength();

        // Once we have the entire command, parse it and reset the state machine
        if( _serialLengthisSet &&
            ( ( _serialIndex - SERIAL_OFFSET ) >= _serialLength ) ) {
            serialFormatToAMRFormat();
            _serialIndex = 0;
            _serialLengthisSet = 0;
        }
    }
    else {
        _serialIndex = 0;
        _serialLengthisSet = 0;
    }
}

void serialConsole()
{
    _headAddr = FLASH_init();
    _headAddr += FLASH_IMAGE_OFFSET;

    while( 1 ) {
        resetSerialTimeOut();
        _hexBuff[_hexIndex] = makeHex( UART_read() );
        if( checkSerialTimeOut() ) return;

        if( _hexIndex == 0 )
            _hexIndex++;
        else if( _hexIndex == 1 ) {
            serialParser( getHex( _hexBuff ) );
            _hexIndex = 0;
        }

        if( _flashComplete ) {
            burnFlashHeader();
            checkFlashImage();
        }
    }
}
