/*
  Written by Warren Woolsey

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <NVM.h>
#include <sam.h>
#include <string.h>

#define MAX_BOOT_SIZE 32768ul
#define MAX_EEPROM_SIZE 16384ul

#define NVM_MAKE_CMD( x ) ( NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD( x ) )
#define NVM_CHECK_ERR NVMCTRL->INTFLAG.bit.ERROR
#define NVM_WAIT_BUSY()                      \
    {                                        \
        while( !NVMCTRL->INTFLAG.bit.READY ) \
            ;                                \
        NVMCTRL->INTFLAG.bit.READY = 1;      \
    }

uint8_t     _paramsLoaded = 0;
NVMParams_t _params;

NVMParams_t getNVMParams()
{
    // Pull the boot loader and EEPROM size configuration fuses
    uint32_t bootBits = ( *( (uint32_t *)NVMCTRL_FUSES_BOOTPROT_ADDR ) &
                          NVMCTRL_FUSES_BOOTPROT_Msk ) >>
                        NVMCTRL_FUSES_BOOTPROT_Pos;
    uint32_t eepBits = ( *( (uint32_t *)NVMCTRL_FUSES_EEPROM_SIZE_ADDR ) &
                         NVMCTRL_FUSES_EEPROM_SIZE_Msk ) >>
                       NVMCTRL_FUSES_EEPROM_SIZE_Pos;

    // Calculate the size of EEPROM and boot based on the fuses, see Data sheet
    // section 20.6.5
    uint8_t divider = 0x1;
    _params.bootSize =
        ( bootBits >= 0x7 ? 0 : MAX_BOOT_SIZE / ( divider << bootBits ) );
    _params.eepromSize =
        ( eepBits >= 0x7 ? 0 : MAX_EEPROM_SIZE / ( divider << eepBits ) );

    // Calculate total memory size based on the PARAM register
    uint8_t pageSizeBits = NVMCTRL->PARAM.bit.PSZ;
    _params.pageSize = ( 0x8 << pageSizeBits );
    uint16_t numPages = NVMCTRL->PARAM.bit.NVMP;
    _params.nvmTotalSize = numPages * _params.pageSize;
    _params.rowSize = _params.pageSize * 4;

    _paramsLoaded = 0x1;
    return _params;
}

void handleNVMError()
{
    NVMCTRL->INTFLAG.bit.ERROR = 1;
}

/* Note: user is responsible for knowing NVM erase procedures and memory spaces
 * associated with the erase row command. See Data see section 20.6.4.4 for
 * more information. 
 */
void eraseRow( uint32_t addr )
{
    if( !_paramsLoaded ) getNVMParams();

    if( addr > _params.nvmTotalSize ) return;
    NVM_WAIT_BUSY();
    NVMCTRL->ADDR.reg = addr / 2;
    NVMCTRL->CTRLA.reg = NVM_MAKE_CMD( NVMCTRL_CTRLA_CMD_ER_Val );
    NVM_WAIT_BUSY();
    if( NVM_CHECK_ERR ) handleNVMError();
}

void writeFlash( const volatile void *flash_ptr, const void *data,
                 uint32_t size )
{
    if( !_paramsLoaded ) getNVMParams();

    // Calculate data boundaries
    size = ( size + 3 ) / 4;
    volatile uint32_t *dst_addr = (volatile uint32_t *)flash_ptr;
    const uint8_t *    src_addr = (uint8_t *)data;

    // Disable automatic page write
    NVMCTRL->CTRLB.bit.MANW = 1;

    // Do writes in pages
    while( size ) {
        // Execute "PBC" Page Buffer Clear
        NVMCTRL->CTRLA.reg = NVM_MAKE_CMD( NVMCTRL_CTRLA_CMD_PBC_Val );
        NVM_WAIT_BUSY();

        // Fill page buffer
        for( uint32_t i = 0; i < ( _params.pageSize / 4 ) && size; i++ ) {
            *dst_addr = *( (uint32_t *)( src_addr ) );
            src_addr += 4;
            dst_addr++;
            size--;
        }

        // Execute "WP" Write Page
        NVMCTRL->CTRLA.reg = NVM_MAKE_CMD( NVMCTRL_CTRLA_CMD_WP_Val );
        NVM_WAIT_BUSY();
    }
}

void readFlash( const volatile void *flash_ptr, void *data, uint32_t size )
{
    if( !_paramsLoaded ) getNVMParams();

    NVM_WAIT_BUSY();
    memcpy( data, (const void *)flash_ptr, size );
    if( NVM_CHECK_ERR ) handleNVMError();
}
