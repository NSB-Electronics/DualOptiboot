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
#ifndef NVM_H_
#define NVM_H_

#include <stdint.h>

typedef struct
{
    uint16_t eepromSize;
    uint16_t bootSize;
    uint32_t nvmTotalSize;
    uint16_t pageSize;
    uint16_t rowSize;
} NVMParams_t;

#ifdef __cplusplus
extern "C" {
#endif

NVMParams_t getNVMParams();
void        handleNVMError();
void        eraseRow( uint32_t addr );
void        writeFlash( const volatile void *flash_ptr, const void *data,
                        uint32_t size );
void readFlash( const volatile void *flash_ptr, void *data, uint32_t size );

#ifdef __cplusplus
}
#endif

#endif /* NVM_H_ */