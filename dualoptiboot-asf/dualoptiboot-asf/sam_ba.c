#include "sam_ba.h"
#include "jump.h"
#include "usb_start.h"
#include <atmel_start.h>
#include <stdlib.h>
#include <string.h>

const char RomBOOT_Version[] = SAM_BA_VERSION;
// X = Chip Erase, Y = Write Buffer, Z = Checksum Buffer, P = Secure Bit Aware
const char RomBOOT_ExtendedCapabilities[] = "[Arduino:XYZP]";

/* Provides one common interface to handle both USART and USB-CDC */
typedef struct
{
    /* send one byte of data */
    int ( *put_c )( int value );
    /* Get one byte */
    int ( *get_c )( void );
    /* Receive buffer not empty */
    bool ( *is_rx_ready )( void );
    /* Send given data (polling) */
    int32_t ( *putdata )( uint8_t *data, uint32_t length );
    /* Get data from comm. device */
    int32_t ( *getdata )( uint8_t *data, uint32_t length );
    /* Send given data (polling) using xmodem (if necessary) */
    int32_t ( *putdata_xmd )( uint8_t *data, uint32_t length );
    /* Get data from comm. device using xmodem (if necessary) */
    int32_t ( *getdata_xmd )( uint8_t *data, uint32_t length );
} t_monitor_if;

#if defined( SAM_BA_UART_ONLY ) || defined( SAM_BA_BOTH_INTERFACES )
/* Initialize structures with function pointers from supported interfaces */
const t_monitor_if uart_if =
    {
        .put_c = serial_putc,
        .get_c = serial_getc,
        //.is_rx_ready = serial_is_rx_ready,
        .putdata = serial_putdata,
        .getdata = serial_getdata,
        .putdata_xmd = serial_putdata_xmd,
        .getdata_xmd = serial_getdata_xmd};
#endif

#if defined( SAM_BA_USBCDC_ONLY ) || defined( SAM_BA_BOTH_INTERFACES )
//Please note that USB doesn't use Xmodem protocol, since USB already includes flow control and data verification
//Data are simply forwarded without further coding.

int _cdc_putc( int value )
{
    cdcdf_acm_write( (uint8_t *)&value, 1 );
    return 0;
}

int _cdc_getc()
{
    int rtn = 0;
    cdcdf_acm_read( (uint8_t *)&rtn, 1 );
    return rtn;
}

const t_monitor_if usbcdc_if =
    {
        .put_c = _cdc_putc,
        .get_c = _cdc_getc,
        //.is_rx_ready = cdc_is_rx_ready,
        .putdata = cdcdf_acm_write,
        .getdata = cdcdf_acm_read,
        .putdata_xmd = cdcdf_acm_write,
        .getdata_xmd = cdcdf_acm_read};
#endif

/* The pointer to the interface object use by the monitor */
t_monitor_if *ptr_monitor_if;

#ifdef SECURE_BY_DEFAULT
bool b_security_enabled = true;
#else
bool b_security_enabled = false;
#endif

/* b_terminal_mode mode (ascii) or hex mode */
volatile bool b_terminal_mode = false;
volatile bool b_sam_ba_interface_usart = false;

static const uint16_t crc16Table[256] =
    {
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
        0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
        0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
        0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
        0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
        0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
        0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
        0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
        0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
        0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
        0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
        0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
        0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
        0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
        0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
        0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
        0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
        0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
        0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
        0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
        0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
        0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
        0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
        0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
        0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
        0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
        0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0};

//*----------------------------------------------------------------------------
//* \brief Compute the CRC
//*----------------------------------------------------------------------------
unsigned short serial_add_crc( char ptr, unsigned short crc )
{
    return ( crc << 8 ) ^ crc16Table[( ( crc >> 8 ) ^ ptr ) & 0xff];
}

void sam_ba_monitor_init( uint8_t com_interface )
{
#if defined( SAM_BA_UART_ONLY ) || defined( SAM_BA_BOTH_INTERFACES )
    //Selects the requested interface for future actions
    if( com_interface == SAM_BA_INTERFACE_USART ) {
        ptr_monitor_if = (t_monitor_if *)&uart_if;
        b_sam_ba_interface_usart = true;
    }
#endif
#if defined( SAM_BA_USBCDC_ONLY ) || defined( SAM_BA_BOTH_INTERFACES )
    if( com_interface == SAM_BA_INTERFACE_USBCDC ) {
        ptr_monitor_if = (t_monitor_if *)&usbcdc_if;
    }
#endif
}

/*
 * Central SAM-BA monitor putdata function using the board LEDs
 */
static uint32_t sam_ba_putdata( t_monitor_if *pInterface, void *data, uint32_t length )
{
    uint32_t result;

    result = pInterface->putdata( (uint8_t *)data, length );

    return result;
}

/*
 * Central SAM-BA monitor getdata function using the board LEDs
 */
static uint32_t sam_ba_getdata( t_monitor_if *pInterface, void *data, uint32_t length )
{
    uint32_t result;

    result = pInterface->getdata( (uint8_t *)data, length );

    return result;
}

/*
 * Central SAM-BA monitor putdata function using the board LEDs
 */
static uint32_t sam_ba_putdata_xmd( t_monitor_if *pInterface, void *data, uint32_t length )
{
    uint32_t result;

    result = pInterface->putdata_xmd( (uint8_t *)data, length );

    return result;
}

/*
 * Central SAM-BA monitor getdata function using the board LEDs
 */
static uint32_t sam_ba_getdata_xmd( t_monitor_if *pInterface, void *data, uint32_t length )
{
    uint32_t result;

    result = pInterface->getdata_xmd( (uint8_t *)data, length );

    return result;
}

/**
 * \brief This function allows data emission by USART
 *
 * \param *data  Data pointer
 * \param length Length of the data
 */
void sam_ba_putdata_term( uint8_t *data, uint32_t length )
{
    uint8_t  temp, buf[12], *data_ascii;
    uint32_t i, int_value;

    if( b_terminal_mode ) {
        if( length == 4 )
            int_value = *(uint32_t *)data;
        else if( length == 2 )
            int_value = *(uint16_t *)data;
        else
            int_value = *(uint8_t *)data;

        data_ascii = buf + 2;
        data_ascii += length * 2 - 1;

        for( i = 0; i < length * 2; i++ ) {
            temp = ( uint8_t )( int_value & 0xf );

            if( temp <= 0x9 )
                *data_ascii = temp | 0x30;
            else
                *data_ascii = temp + 0x37;

            int_value >>= 4;
            data_ascii--;
        }
        buf[0] = '0';
        buf[1] = 'x';
        buf[length * 2 + 2] = '\n';
        buf[length * 2 + 3] = '\r';
        sam_ba_putdata( ptr_monitor_if, buf, length * 2 + 4 );
    }
    else
        sam_ba_putdata( ptr_monitor_if, data, length );
    return;
}

#ifndef SECURE_BY_DEFAULT
volatile uint32_t sp;
void              call_applet( uint32_t address )
{
    if( b_security_enabled ) {
        return;
    }

    uint32_t app_start_address;

    __disable_irq();

    sp = __get_MSP();

    /* Rebase the Stack Pointer */
    __set_MSP( *(uint32_t *)address );

    /* Load the Reset Handler address of the application */
    app_start_address = *(uint32_t *)( address + 4 );

    /* Jump to application Reset Handler in the application */
    asm( "bx %0" ::"r"( app_start_address ) );
}
#endif

uint32_t current_number;
uint32_t erased_from = 0;
uint32_t i, length;
uint8_t  command, *ptr_data, *ptr, data[SIZEBUFMAX];
uint8_t  j;
uint32_t u32tmp;

uint32_t PAGE_SIZE, PAGES, MAX_FLASH;

// Prints a 32-bit integer in hex.
static void put_uint32( uint32_t n )
{
    char buff[8];
    int  i;
    for( i = 0; i < 8; i++ ) {
        int d = n & 0XF;
        n = ( n >> 4 );

        buff[7 - i] = d > 9 ? 'A' + d - 10 : '0' + d;
    }
    sam_ba_putdata( ptr_monitor_if, buff, 8 );
}

#ifdef ENABLE_JTAG_LOAD
static uint32_t offset = __UINT32_MAX__;
static bool     flashNeeded = false;
#endif

static void sam_ba_monitor_loop( void )
{
    length = sam_ba_getdata( ptr_monitor_if, data, SIZEBUFMAX );
    ptr = data;

    for( i = 0; i < length; i++, ptr++ ) {
        if( *ptr == 0xff ) continue;

        if( *ptr == '#' ) {
            if( b_terminal_mode ) {
                sam_ba_putdata( ptr_monitor_if, "\n\r", 2 );
            }
            if( command == 'S' ) // Write memory (normally RAM, but might be flash, if client handles the Flash MCU commands?)
            {
                //Check if some data are remaining in the "data" buffer
                if( length > i ) {
                    //Move current indexes to next avail data (currently ptr points to "#")
                    ptr++;
                    i++;

                    //We need to add first the remaining data of the current buffer already read from usb
                    //read a maximum of "current_number" bytes
                    if( ( length - i ) < current_number ) {
                        u32tmp = ( length - i );
                    }
                    else {
                        u32tmp = current_number;
                    }

                    memcpy( ptr_data, ptr, u32tmp );
                    i += u32tmp;
                    ptr += u32tmp;
                    j = u32tmp;
                }
                //update i with the data read from the buffer
                i--;
                ptr--;
                //Do we expect more data ?
                if( j < current_number )
                    sam_ba_getdata_xmd( ptr_monitor_if, ptr_data, current_number - j );

                __asm( "nop" );
            }
            else if( command == 'R' ) // Read memory (flash or RAM)
            {
                // Flash memory starts at address 0 and runs to flash size 0x40000 (256 KByte)

                // Internal RWW section is at adress 0x400000. RWW is flash used for EEPROM emulation. Will not let anyone read that, when in secure mode, either.
                // Bootloader ends at 0x1FFF, so user programs start at 0x2000
                // RAM starts at 0x20000000, so redirect FLASH reads into RAM reads, when in secure mode
                if( b_security_enabled && ( (uint32_t)ptr_data >= 0x0000 && (uint32_t)ptr_data < 0x20000000 ) ) {
                    ptr_data = (uint8_t *)0x20005000;
                }

                sam_ba_putdata_xmd( ptr_monitor_if, ptr_data, current_number );
            }
            else if( command == 'O' ) // write byte
            {
                *ptr_data = (char)current_number;
            }
            else if( command == 'H' ) // Write half word
            {
                *( (uint16_t *)ptr_data ) = (uint16_t)current_number;
            }
            else if( command == 'W' ) // Write word
            {
                *( (int *)ptr_data ) = current_number;
            }
            else if( command == 'o' ) // Read byte
            {
                // Flash memory starts at address 0 and runs to flash size 0x40000 (256 KByte). RAM starts at 0x20000000.
                // Intern RWW section is at adress 0x400000. RWW is flash used for EEPROM emulation. Will not let anyone read that, when in secure mode, either.
                // BOSSA reads address 0 to check something, but using read word instead of read byte, but in any case allow reading first byte
                // Bootloader ends at 0x1FFF, so user programs start at 0x2000
                if( b_security_enabled && ( (uint32_t)ptr_data > 0x0003 && (uint32_t)ptr_data < 0x20000000 ) ) {
                    ptr_data = (uint8_t *)&current_number;
                }

                sam_ba_putdata_term( ptr_data, 1 );
            }
            else if( command == 'h' ) // Read half word
            {
                // Flash memory starts at address 0 and runs to flash size 0x40000 (256 KByte). RAM starts at 0x20000000.
                // Intern RWW section is at adress 0x400000. RWW is flash used for EEPROM emulation. Will not let anyone read that, when in secure mode, either.
                // BOSSA reads address 0 to check something, but using read word instead of read byte, but in any case allow reading first byte
                // Bootloader ends at 0x1FFF, so user programs start at 0x2000
                if( b_security_enabled && ( (uint32_t)ptr_data > 0x0003 && (uint32_t)ptr_data < 0x20000000 ) ) {
                    current_number = 0;
                }
                else {
                    current_number = *( (uint16_t *)ptr_data );
                }

                sam_ba_putdata_term( (uint8_t *)&current_number, 2 );
            }
            else if( command == 'w' ) // Read word
            {
                // Flash memory starts at address 0 and runs to flash size 0x40000 (256 KByte). RAM starts at 0x20000000.
                // Intern RWW section is at adress 0x400000. RWW is flash used for EEPROM emulation. Will not let anyone read that, when in secure mode, either.
                // BOSSA reads address 0 to check something, but using read word instead of read byte, but in any case allow reading first byte
                // Bootloader ends at 0x1FFF, so user programs start at 0x2000
                if( b_security_enabled && ( (uint32_t)ptr_data > 0x0003 && (uint32_t)ptr_data < 0x20000000 ) ) {
                    current_number = 0;
                }
                else {
                    current_number = *( (uint32_t *)ptr_data );
                }

                sam_ba_putdata_term( (uint8_t *)&current_number, 4 );
            }
#ifndef SECURE_BY_DEFAULT
            else if( !b_security_enabled && command == 'G' ) // Execute code. Will not allow when security is enabled.
            {
                call_applet( current_number );
                /* Rebase the Stack Pointer */
                __set_MSP( sp );
                __enable_irq();
                if( b_sam_ba_interface_usart ) {
                    ptr_monitor_if->put_c( 0x6 );
                }
            }
#endif
            else if( command == 'T' ) // Turn on terminal mode
            {
                b_terminal_mode = 1;
                sam_ba_putdata( ptr_monitor_if, "\n\r", 2 );
            }
            else if( command == 'N' ) // Turn off terminal mode
            {
                if( b_terminal_mode == 0 ) {
                    sam_ba_putdata( ptr_monitor_if, "\n\r", 2 );
                }
                b_terminal_mode = 0;
            }
            else if( command == 'V' ) // Read version information
            {
                sam_ba_putdata( ptr_monitor_if, "v", 1 );
                sam_ba_putdata( ptr_monitor_if, (uint8_t *)RomBOOT_Version, strlen( RomBOOT_Version ) );
                sam_ba_putdata( ptr_monitor_if, " ", 1 );
                sam_ba_putdata( ptr_monitor_if, (uint8_t *)RomBOOT_ExtendedCapabilities, strlen( RomBOOT_ExtendedCapabilities ) );
                ptr = (uint8_t *)&( " " __DATE__ " " __TIME__ "\n\r" );
                sam_ba_putdata( ptr_monitor_if, ptr, strlen( (char *)ptr ) );
            }
            else if( command == 'X' ) // Erase flash
            {
                // Syntax: X[ADDR]#
                // Erase the flash memory starting from ADDR to the end of flash.

                // Note: the flash memory is erased in ROWS, that is in block of 4 pages.
                //       Even if the starting address is the last byte of a ROW the entire
                //       ROW is erased anyway.

                // BOSSAC.exe always erase with 0x2000 as argument, but an attacker might try to erase just parts of the flash, to be able to copy or analyze the untouched parts.
                // To mitigate this, always erase all sketch flash, that is, starting from address 0x2000. This butloader always assume 8 KByte for itself, and sketch starting at 0x2000.
                flash_erase( &INTERNAL_FLASH, b_security_enabled ? APP_START_ADDR : current_number, 1 );
                // Notify command completed
                sam_ba_putdata( ptr_monitor_if, "X\n\r", 3 );
            }
            else if( command == 'Y' ) // Write buffer to flash
            {
                // This command writes the content of a buffer in SRAM into flash memory.

                // Syntax: Y[ADDR],0#
                // Set the starting address of the SRAM buffer.

                // Syntax: Y[ROM_ADDR],[SIZE]#
                // Write the first SIZE bytes from the SRAM buffer (previously set) into
                // flash memory starting from address ROM_ADDR

                static uint32_t *src_buff_addr = NULL;

                if( current_number == 0 ) {
                    // Set buffer address
                    src_buff_addr = (uint32_t *)ptr_data;
                }
                else {
                    if( b_security_enabled && erased_from != APP_START_ADDR ) {
                        // To mitigate that an attacker might not use the ordinary BOSSA method of erasing flash before programming,
                        // always erase flash, if it hasn't been done already.
                        flash_erase( &INTERNAL_FLASH, APP_START_ADDR, 1 );
                    }

                    // Write to flash
                    uint32_t  size = current_number / 4;
                    uint32_t *src_addr = src_buff_addr;
                    uint32_t *dst_addr = (uint32_t *)ptr_data;

#ifdef ENABLE_JTAG_LOAD

                    if( (uint32_t)dst_addr == 0x40000 ) {
                        if( jtagInit() != 0 ) {
                            // fail!
                            sam_ba_putdata( ptr_monitor_if, "y\n\r", 3 );
                            return;
                        }

                        // content of the first flash page:
                        // offset (32) : length(32) : sha256sum(256) : type (32) : force (32) = 48 bytes
                        // for every section; check last sector of the flash to understand if reflash is needed
                        externalFlashSignatures data[3];
                        jtagFlashReadBlock( LAST_FLASH_PAGE, sizeof( data ), (uint8_t *)data );
                        externalFlashSignatures *newData = (externalFlashSignatures *)src_addr;
                        for( int k = 0; k < 3; k++ ) {
                            if( newData[k].force != 0 ) {
                                offset = newData[k].offset;
                                flashNeeded = true;
                                break;
                            }
                            if( ( data[k].type == newData[k].type ) || ( data[k].type == 0xFFFFFFFF ) ) {
                                if( newData[k].offset < offset ) {
                                    offset = newData[k].offset;
                                }
                                if( memcmp( data[k].sha256sum, newData[k].sha256sum, 32 ) != 0 ) {
                                    flashNeeded = true;
                                    break;
                                }
                            }
                        }

                        // merge old page and new page
                        for( int k = 0; k < 3; k++ ) {
                            if( newData[k].type != 0xFFFFFFFF ) {
                                memcpy( &data[k], &newData[k], sizeof( newData[k] ) );
                            }
                        }

                        jtagFlashEraseBlock( SCRATCHPAD_FLASH_PAGE );
                        // write first page to SCRATCHPAD_FLASH_PAGE (to allow correct verification)
                        for( int j = 0; j < size; ) {
                            jtagFlashWriteBlock( SCRATCHPAD_FLASH_PAGE + j * 4, 512, (uint32_t *)&src_addr[j] );
                            j += 128;
                        }

                        // write real structure with checksums to LAST_FLASH_PAGE
                        jtagFlashWriteBlock( LAST_FLASH_PAGE, sizeof( data ), (uint32_t *)data );
                        goto end;
                    }

                    if( (uint32_t)dst_addr >= 0x41000 ) {

                        if( flashNeeded == false ) {
                            goto end;
                        }

                        uint32_t rebasedAddress = (uint32_t)dst_addr - 0x41000 + offset;
                        if( rebasedAddress % 0x10000 == 0 ) {
                            jtagFlashEraseBlock( rebasedAddress );
                        }

                        for( int j = 0; j < size; ) {
                            jtagFlashWriteBlock( rebasedAddress + j * 4, 512, (uint32_t *)&src_addr[j] );
                            j += 128;
                        }
                        goto end;
                    }
#endif
                    // Set automatic page write
                    NVMCTRL->CTRLB.bit.MANW = 0;

                    // Do writes in pages
                    while( size ) {
                        // Execute "PBC" Page Buffer Clear
                        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
                        while( NVMCTRL->INTFLAG.bit.READY == 0 )
                            ;

                        // Fill page buffer
                        uint32_t i;
                        for( i = 0; i < ( PAGE_SIZE / 4 ) && i < size; i++ ) {
                            dst_addr[i] = src_addr[i];
                        }

                        // Execute "WP" Write Page
                        //NVMCTRL->ADDR.reg = ((uint32_t)dst_addr) / 2;
                        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;
                        while( NVMCTRL->INTFLAG.bit.READY == 0 )
                            ;

                        // Advance to next page
                        dst_addr += i;
                        src_addr += i;
                        size -= i;
                    }
                }

#ifdef ENABLE_JTAG_LOAD
            end:
#endif

                // Notify command completed
                sam_ba_putdata( ptr_monitor_if, "Y\n\r", 3 );
            }
            else if( command == 'Z' ) // Calculate CRC16
            {
                // This command calculate CRC for a given area of memory.
                // It's useful to quickly check if a transfer has been done
                // successfully.

                // Syntax: Z[START_ADDR],[SIZE]#
                // Returns: Z[CRC]#

                uint8_t *data;
                uint32_t size = current_number;
                uint16_t crc = 0;
                uint32_t i = 0;

#ifdef ENABLE_JTAG_LOAD
                uint8_t buf[4096];
#endif

#ifdef ENABLE_JTAG_LOAD
                if( (uint32_t)ptr_data == 0x40000 ) {
                    data = (uint8_t *)buf;
                    for( int j = 0; j < size; ) {
                        jtagFlashReadBlock( SCRATCHPAD_FLASH_PAGE + j, 256, &data[j] );
                        j += 256;
                    }
                }
                else if( (uint32_t)ptr_data >= 0x41000 ) {
                    data = (uint8_t *)buf;
                    for( int j = 0; j < size; ) {
                        jtagFlashReadBlock( (uint32_t)ptr_data + offset - 0x41000 + j, 256, &data[j] );
                        j += 256;
                    }
                }
                else {
                    data = (uint8_t *)ptr_data;
                }
#else
                data = (uint8_t *)ptr_data;
#endif

                for( i = 0; i < size; i++ )
                    crc = serial_add_crc( *data++, crc );

                // Send response
                sam_ba_putdata( ptr_monitor_if, "Z", 1 );
                put_uint32( crc );
                sam_ba_putdata( ptr_monitor_if, "#\n\r", 3 );
            }

            command = 'z';
            current_number = 0;

            if( b_terminal_mode ) {
                sam_ba_putdata( ptr_monitor_if, ">", 1 );
            }
        }
        else {
            if( ( '0' <= *ptr ) && ( *ptr <= '9' ) ) {
                current_number = ( current_number << 4 ) | ( *ptr - '0' );
            }
            else if( ( 'A' <= *ptr ) && ( *ptr <= 'F' ) ) {
                current_number = ( current_number << 4 ) | ( *ptr - 'A' + 0xa );
            }
            else if( ( 'a' <= *ptr ) && ( *ptr <= 'f' ) ) {
                current_number = ( current_number << 4 ) | ( *ptr - 'a' + 0xa );
            }
            else if( *ptr == ',' ) {
                ptr_data = (uint8_t *)current_number;
                current_number = 0;
            }
            else {
                command = *ptr;
                current_number = 0;
            }
        }
    }
}

/**
 * \brief This function starts the SAM-BA monitor.
 */
void sam_ba_monitor_run( void )
{
    uint32_t pageSizes[] = {8, 16, 32, 64, 128, 256, 512, 1024};
    PAGE_SIZE = pageSizes[NVMCTRL->PARAM.bit.PSZ];
    PAGES = NVMCTRL->PARAM.bit.NVMP;
    MAX_FLASH = PAGE_SIZE * PAGES;

#ifdef SECURE_BY_DEFAULT
    b_security_enabled = true;
#else
    b_security_enabled = NVMCTRL->STATUS.bit.SB != 0;
#endif

    ptr_data = NULL;
    command = 'z';
    while( 1 ) {
        sam_ba_monitor_loop();
    }
}
