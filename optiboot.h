#ifndef OPTIBOOT__h
#define OPTIBOOT__h

/* ========================================================================== *
 * Pre-processor utilities
 * ========================================================================== */
#define MAKESTR( a ) #a
#define MAKEVER( a, b ) MAKESTR( a * 256 + b )

/* ========================================================================== *
 * Hard-coded Definitions
 * ========================================================================== */
#define OPTIBOOT_MAJVER 5
#define OPTIBOOT_MINVER 0

#define EXPECTED_JDEC 0x42

#ifndef __AVR_ATmega328P__
#error "Unsupported"
#endif

/* ========================================================================== *
 * Options
 * ========================================================================== */

// #define LED_DATA_FLASH  1

/* ========================================================================== *
 * SPI Flash Defintions
 * ========================================================================== */
#define SPI_MODE0 0x00
#define SPI_MODE_MASK 0x0C    /* CPOL = bit 3, CPHA = bit 2 on SPCR */
#define SPI_CLOCK_MASK 0x03   /* SPR1 = bit 1, SPR0 = bit 0 on SPCR */
#define SPI_2XCLOCK_MASK 0x01 /* SPI2X = bit 0 on SPSR */
#define SPI_CLOCK_DIV2 0x04

#define FLASH_SS_DDR DDRB
#define FLASH_SS_PORT PORTB
#define FLASH_SS PINB0
#define RFM_SS PINB2

#define FLASH_SELECT                           \
    {                                          \
        FLASH_SS_PORT &= ~( _BV( FLASH_SS ) ); \
    }
#define FLASH_UNSELECT                    \
    {                                     \
        FLASH_SS_PORT |= _BV( FLASH_SS ); \
    }

#define SPIFLASH_STATUSWRITE 0x01      /* write status register */
#define SPIFLASH_STATUSREAD 0x05       /* read status register */
#define SPIFLASH_WRITEENABLE 0x06      /* write enable */
#define SPIFLASH_ARRAYREADLOWFREQ 0x03 /* read array (low frequency) */
#define SPIFLASH_BLOCKERASE_4K 0x20    /* erase one 4K block of flash memory */
#define SPIFLASH_BLOCKERASE_32K 0x52   /* erase one 32K block of flash memory */
#define SPIFLASH_BLOCKERASE_64K 0xD8   /* erase one 32K block of flash memory */
#define SPIFLASH_JEDECID 0x9F          /* read JEDEC ID */

/* uncomment to enable Serial debugging */
// #define DEBUG_ON

/* ========================================================================== *
 * Defaults
 * ========================================================================== */

#ifndef LED_START_FLASHES
#define LED_START_FLASHES 0
#endif

#ifndef BAUD_RATE
#error "bad baudrate"
#endif

#ifndef UART
#define UART 0
#endif

/* ========================================================================== *
 * Baudrate
 * ========================================================================== */
#define BAUD_SETTING \
    ( ( ( F_CPU + BAUD_RATE * 4L ) / ( ( BAUD_RATE * 8L ) ) ) - 1 )
#define BAUD_ACTUAL ( F_CPU / ( 8 * ( ( BAUD_SETTING ) + 1 ) ) )
#define BAUD_ERROR ( ( 100 * ( BAUD_RATE - BAUD_ACTUAL ) ) / BAUD_RATE )

/* baud rate slow check */
#if( F_CPU + BAUD_RATE * 4L ) / ( BAUD_RATE * 8L ) - 1 > 250
#error Unachievable baud rate (too slow) BAUD_RATE
#endif

/* baud rate fastn check */
#if( F_CPU + BAUD_RATE * 4L ) / ( BAUD_RATE * 8L ) - 1 < 3
#error Unachievable baud rate (too fast) BAUD_RATE
#endif

/* ========================================================================== *
 * WDT
 * ========================================================================== */
/* Watchdog settings */
#define WATCHDOG_OFF ( 0 )
#define WATCHDOG_16MS ( _BV( WDE ) )
#define WATCHDOG_32MS ( _BV( WDP0 ) | _BV( WDE ) )
#define WATCHDOG_64MS ( _BV( WDP1 ) | _BV( WDE ) )
#define WATCHDOG_125MS ( _BV( WDP1 ) | _BV( WDP0 ) | _BV( WDE ) )
#define WATCHDOG_250MS ( _BV( WDP2 ) | _BV( WDE ) )
#define WATCHDOG_500MS ( _BV( WDP2 ) | _BV( WDP0 ) | _BV( WDE ) )
#define WATCHDOG_1S ( _BV( WDP2 ) | _BV( WDP1 ) | _BV( WDE ) )
#define WATCHDOG_2S ( _BV( WDP2 ) | _BV( WDP1 ) | _BV( WDP0 ) | _BV( WDE ) )
#define WATCHDOG_4S ( _BV( WDP3 ) | _BV( WDE ) )
#define WATCHDOG_8S ( _BV( WDP3 ) | _BV( WDP0 ) | _BV( WDE ) )

/* ========================================================================== *
 * NRWW memory
 * ========================================================================== */
/* Addresses below NRWW (Non-Read-While-Write) can be programmed while
 * continuing to run code from flash, slightly speeding up programming time.
 * Beware that Atmel data sheets specify this as a WORD address, while optiboot
 * will be comparing against a 16-bit byte address. This means that on a 128kB
 * chip the upper part of the lower 64k will get NRWW processing as well, even
 * though it doesn't need it. That's OK. In fact, you can disable the
 * overlapping processing for a part entirely by setting NRWWSTART to zero. This
 * reduces code space a bit, at the expense of being slightly slower, overall.
 *
 * RAMSTART should be self-explanatory. It's bigger on chips with a lot of
 * peripheral registers.
 **/
#if defined( __AVR_ATmega168__ )
#define RAMSTART ( 0x100 )
#define NRWWSTART ( 0x3800 )
#elif defined( __AVR_ATmega328P__ ) || defined( __AVR_ATmega32__ )
#define RAMSTART ( 0x100 )
#define NRWWSTART ( 0x7000 )
#elif defined( __AVR_ATmega644P__ )
#define RAMSTART ( 0x100 )
#define NRWWSTART ( 0xE000 )
/* correct for a bug in avr-libc */
#undef SIGNATURE_2
#define SIGNATURE_2 0x0A
#elif defined( __AVR_ATmega1284P__ )
#define RAMSTART ( 0x100 )
#define NRWWSTART ( 0xE000 )
#elif defined( __AVR_ATtiny84__ )
#define RAMSTART ( 0x100 )
#define NRWWSTART ( 0x0000 )
#elif defined( __AVR_ATmega1280__ )
#define RAMSTART ( 0x200 )
#define NRWWSTART ( 0xE000 )
#elif defined( __AVR_ATmega8__ ) || defined( __AVR_ATmega88__ )
#define RAMSTART ( 0x100 )
#define NRWWSTART ( 0x1800 )
#endif

/* ========================================================================== *
 * Initializer Macros
 * ========================================================================== */
/* C zero initialises all global variables. However, that requires program
 * space. These definitions allow us to drop the zero init code, saving memory.
 */
#define buff ( (uint8_t *)( RAMSTART ) )
#ifdef VIRTUAL_BOOT_PARTITION
#define rstVect ( *(uint16_t *)( RAMSTART + SPM_PAGESIZE * 2 + 4 ) )
#define wdtVect ( *(uint16_t *)( RAMSTART + SPM_PAGESIZE * 2 + 6 ) )
#endif

/* ========================================================================== *
 * UART Definitions
 * ========================================================================== */
/* Handle devices with up to 4 uarts (eg m1280.) Note that mega8/m32 still needs
 * special handling, because ubrr is handled differently.
 **/
#if UART == 0
#define UART_SRA UCSR0A
#define UART_SRB UCSR0B
#define UART_SRC UCSR0C
#define UART_SRL UBRR0L
#define UART_UDR UDR0
#elif UART == 1
#if !defined( UDR1 )
#error UART == 1, but no UART1 on device
#endif
#define UART_SRA UCSR1A
#define UART_SRB UCSR1B
#define UART_SRC UCSR1C
#define UART_SRL UBRR1L
#define UART_UDR UDR1
#elif UART == 2
#if !defined( UDR2 )
#error UART == 2, but no UART2 on device
#endif
#define UART_SRA UCSR2A
#define UART_SRB UCSR2B
#define UART_SRC UCSR2C
#define UART_SRL UBRR2L
#define UART_UDR UDR2
#elif UART == 3
#if !defined( UDR1 )
#error UART == 3, but no UART3 on device
#endif
#define UART_SRA UCSR3A
#define UART_SRB UCSR3B
#define UART_SRC UCSR3C
#define UART_SRL UBRR3L
#define UART_UDR UDR3
#endif

#endif /* OPTIBOOT__h */
