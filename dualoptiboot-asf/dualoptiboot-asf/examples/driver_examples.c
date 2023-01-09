/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include "driver_examples.h"
#include "driver_init.h"
#include "utils.h"

static uint8_t src_data[128];
static uint8_t chk_data[128];
/**
 * Example of using INTERNAL_FLASH to read and write Flash main array.
 */
void INTERNAL_FLASH_example(void)
{
	uint32_t page_size;
	uint16_t i;

	/* Init source data */
	page_size = flash_get_page_size(&INTERNAL_FLASH);

	for (i = 0; i < page_size; i++) {
		src_data[i] = i;
	}

	/* Write data to flash */
	flash_write(&INTERNAL_FLASH, 0x3200, src_data, page_size);

	/* Read data from flash */
	flash_read(&INTERNAL_FLASH, 0x3200, chk_data, page_size);
}

/**
 * Example of using M0_SPI_FLASH to write "Hello World" using the IO abstraction.
 */
static uint8_t example_M0_SPI_FLASH[12] = "Hello World!";

void M0_SPI_FLASH_example(void)
{
	struct io_descriptor *io;
	spi_m_sync_get_io_descriptor(&M0_SPI_FLASH, &io);

	spi_m_sync_enable(&M0_SPI_FLASH);
	io_write(io, example_M0_SPI_FLASH, 12);
}

/**
 * Example of using RED_BOARD_SPI_FLASH to write "Hello World" using the IO abstraction.
 */
static uint8_t example_RED_BOARD_SPI_FLASH[12] = "Hello World!";

void RED_BOARD_SPI_FLASH_example(void)
{
	struct io_descriptor *io;
	spi_m_sync_get_io_descriptor(&RED_BOARD_SPI_FLASH, &io);

	spi_m_sync_enable(&RED_BOARD_SPI_FLASH);
	io_write(io, example_RED_BOARD_SPI_FLASH, 12);
}
