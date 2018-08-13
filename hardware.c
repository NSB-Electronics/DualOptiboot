#include <hardware.h>

#define GCLK_WAIT_SYNC while( GCLK->STATUS.bit.SYNCBUSY )

inline void SPI_init()
{
    SET_PIN_OUTMODE( RFM_SS );
    SET_PIN_OUTMODE( FLASH_SS );
    OUTPUT_HIGH( RFM_SS );
    OUTPUT_HIGH( FLASH_SS );

    SET_ODD_PIN_PERIPH( MISO, 2 );
    SET_ODD_PIN_PERIPH( SCK, 2 );
    SET_EVEN_PIN_PERIPH( MOSI, 2 );

    // Enable the bus clock
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( GCLK_CLKCTRL_ID_SERCOM1_CORE_Val ) |
                        GCLK_CLKCTRL_GEN( GCLK_CLKCTRL_GEN_GCLK0_Val ) |
                        GCLK_CLKCTRL_CLKEN;
    GCLK_WAIT_SYNC;

    PM->APBCMASK.reg |= PM_APBCMASK_SERCOM1;

    // Reset
    SERCOM1->SPI.CTRLA.bit.SWRST = 1;
    while( SERCOM1->SPI.CTRLA.bit.SWRST || SERCOM1->SPI.STATUS.bit.SYNCBUSY )
        ;

    // Setting the CTRLA register
    SERCOM1->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_MODE_SPI_MASTER |
                             SERCOM_SPI_CTRLA_DOPO( SPI_DATA_OUTPUT_PAD ) |
                             SERCOM_SPI_CTRLA_DIPO( SPI_DATA_INPUT_PAD );

    // Setting the CTRLB register
    SERCOM1->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_RXEN; // Active the SPI receiver.
    while( SERCOM1->SPI.STATUS.bit.SYNCBUSY )
        ;

    // Set the baud rate and enable
    SERCOM1->SPI.BAUD.reg = FCPU / ( 2 * SPI_BAUD_RATE ) - 1;
    SERCOM1->SPI.CTRLA.bit.ENABLE = 1;
    while( SERCOM1->SPI.STATUS.bit.SYNCBUSY )
        ;
}

inline void UART_init()
{
    SET_ODD_PIN_PERIPH( UART_RX, 2 );
    SET_EVEN_PIN_PERIPH( UART_TX, 2 );

    // Enable the bus clock
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( GCLK_CLKCTRL_ID_SERCOM3_CORE_Val ) |
                        GCLK_CLKCTRL_GEN( GCLK_CLKCTRL_GEN_GCLK0_Val ) |
                        GCLK_CLKCTRL_CLKEN;
    GCLK_WAIT_SYNC;

    PM->APBCMASK.reg |= PM_APBCMASK_SERCOM3;

    // Reset
    SERCOM3->USART.CTRLA.bit.SWRST = 1;
    while( SERCOM3->USART.CTRLA.bit.SWRST ||
           SERCOM3->USART.STATUS.bit.SYNCBUSY )
        ;

    // Setting CTRLA register
    SERCOM3->USART.CTRLA.reg =
        SERCOM_USART_CTRLA_DORD | SERCOM_USART_CTRLA_RXPO( UART_RX_PAD ) |
        SERCOM_USART_CTRLA_TXPO |
        SERCOM_USART_CTRLA_MODE( SERCOM_USART_CTRLA_MODE_USART_INT_CLK_Val );
    while( SERCOM3->USART.STATUS.bit.SYNCBUSY )
        ;

    // Setting CTRLB register
    SERCOM3->USART.CTRLB.reg =
        SERCOM_USART_CTRLB_RXEN | SERCOM_USART_CTRLB_TXEN;
    while( SERCOM3->USART.STATUS.bit.SYNCBUSY )
        ;

    // Set baud rate
    uint64_t ratio = 1048576;
    ratio *= UART_BAUD_RATE;
    ratio /= FCPU;
    SERCOM3->USART.BAUD.reg = ( uint16_t )( 65536 - ratio );

    // Enable UART
    SERCOM3->USART.CTRLA.bit.ENABLE = 1;
    while( SERCOM3->USART.STATUS.bit.SYNCBUSY )
        ;
}

void initHardware()
{
    // Launch the 8 MHz oscillator
    SYSCTRL->OSC8M.bit.ONDEMAND = 0;

#if( FCPU == 8000000 )
    SYSCTRL->OSC8M.bit.PRESC = 0;
#elif( FCPU == 4000000 )
    SYSCTRL->OSC8M.bit.PRESC = 1;
#elif( FCPU == 2000000 )
    SYSCTRL->OSC8M.bit.PRESC = 2;
#elif( FCPU == 1000000 )
    SYSCTRL->OSC8M.bit.PRESC = 3;
#else
#error "Bad CPU Frequency"
#endif /* FCPU */

    SYSCTRL->OSC8M.bit.ENABLE = 1;
    while( !SYSCTRL->PCLKSR.bit.OSC8MRDY )
        ;

    // Hook the 8 MHz oscillator up to the main clock source
    uint32_t genDivReg = 0;
    uint32_t genCtrlReg = 0;

    genDivReg =
        GCLK_GENDIV_ID( GCLK_GENDIV_ID_GCLK0_Val ) | GCLK_GENDIV_DIV( 0 );
    GCLK->GENDIV.reg = genDivReg;
    GCLK_WAIT_SYNC;

    genCtrlReg = GCLK_GENCTRL_ID( GCLK_GENDIV_ID_GCLK0_Val ) |
                 GCLK_GENCTRL_SRC( GCLK_GENCTRL_SRC_OSC8M_Val ) |
                 GCLK_GENCTRL_IDC | GCLK_GENCTRL_GENEN;
    GCLK->GENCTRL.reg = genCtrlReg;
    GCLK_WAIT_SYNC;

    // Setup the GPIOs
    PM->APBBMASK.reg |= PM_APBBMASK_PORT;

    // Initialize serial communications
    UART_init();
    SPI_init();
}

uint8_t SPI_transfer( uint8_t _data )
{
    SERCOM1->SPI.DATA.bit.DATA = _data;
    while( SERCOM1->SPI.INTFLAG.bit.RXC == 0 )
        ;
    return SERCOM1->SPI.DATA.bit.DATA;
}

void UART_write( uint8_t _data )
{
    while( !SERCOM3->USART.INTFLAG.bit.DRE )
        ;
    SERCOM3->USART.DATA.reg = (uint16_t)_data;
}

uint8_t UART_read()
{
    while( !SERCOM3->USART.INTFLAG.bit.RXC )
        ;
    return SERCOM3->USART.DATA.reg;
}

void cleanUp()
{}