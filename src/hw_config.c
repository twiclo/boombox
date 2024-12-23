#include "hw_config.h"

// Hardware Configuration of SPI "objects"
static spi_t spis[] = {  // One for each SPI.
	{
		.hw_inst = spi0,  // SPI component
		.miso_gpio = 16, // GPIO number (not pin number)
		.mosi_gpio = 19,
		.sck_gpio = 18,
		.baud_rate = 12500 * 1000,  
	}
};

// Hardware Configuration of the SD Card "objects"
static sd_card_t sd_cards[] = {  // One for each SD card
	{
		.pcName = "0:",   // Name used to mount device
		.spi = &spis[0],  // Pointer to the SPI driving this card
		.ss_gpio = 17,    // The SPI slave select GPIO for this SD card
		.use_card_detect = false,
		/* .card_detect_gpio = ,   // Card detect */
		.card_detected_true = -1  // What the GPIO read returns when a card is
														 // present. Use -1 if there is no card detect.
	}};

size_t sd_get_num() { return count_of(sd_cards); }
sd_card_t *sd_get_by_num(size_t num) {
	if (num <= sd_get_num()) {
		return &sd_cards[num];
	} else {
		return NULL;
	}
}

size_t spi_get_num() { return count_of(spis); }
spi_t *spi_get_by_num(size_t num) {
	if (num <= sd_get_num()) {
		return &spis[num];
	} else {
		return NULL;
	}
}
