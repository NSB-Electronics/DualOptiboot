#ifndef EXTFLASH_H_
#define EXTFLASH_H_

#include <stdint.h>

#define SPIFLASH_STATUSWRITE 0x01      /* write status register */
#define SPIFLASH_STATUSREAD 0x05       /* read status register */
#define SPIFLASH_WRITEENABLE 0x06      /* write enable */
#define SPIFLASH_ARRAYREADLOWFREQ 0x03 /* read array (low frequency) */
#define SPIFLASH_BLOCKERASE_4K 0x20    /* erase one 4K block of flash memory */
#define SPIFLASH_BLOCKERASE_32K 0x52   /* erase one 32K block of flash memory */
#define SPIFLASH_BLOCKERASE_64K 0xD8   /* erase one 32K block of flash memory */
#define SPIFLASH_JEDECID 0x9F          /* read JEDEC ID */

uint8_t FLASH_busy();
void    FLASH_command( uint8_t cmd, uint8_t isWrite );
uint8_t FLASH_readByte( uint32_t addr );
void CheckFlashImage();

#endif /* EXTFLASH_H_ */