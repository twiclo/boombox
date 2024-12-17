#include <string.h>
#include "ff.h" /* Obtains integer types */
#include "ff_stdio.h" /* Obtains integer types */
#include "diskio.h" /* Declarations of disk functions */
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/i2c.h"

typedef struct __attribute__((packed)) {
	char riff_id[4];
	uint32_t filesize;
	char filetype[4];
	char chunk_mark[4];
	uint32_t chunk_size;
	uint16_t format_type;
	uint16_t channels;
	uint32_t sample_rate;
	uint32_t bits_per_sec;
	uint16_t block_align;
	uint16_t bits_per_sample;
	char data_header[4];
	uint32_t data_size;
} WaveHeader;

int main() {
	FRESULT fr;
	FATFS fs;
	FIL file;
	char full_path[50];

	char cart_path[12];
	DIR cart_dir;
	FILINFO file_info;
	WaveHeader header;

	uint32_t selection = 0;
	uint32_t mag_gpios[] = {6, 7, 8, 9, 10};

	cyw43_arch_init();
	stdio_init_all();

	printf("\r\nPress 'enter' to start.\r\n");
	while (true) {
		char buf[2];
		buf[0] = getchar();
		if ((buf[0] == '\r') || (buf[0] == '\n')) {
			break;
		}
	}

	// Init power off gpio
	gpio_init(1);
	gpio_set_dir(1, GPIO_OUT);

	for (uint32_t i = 0; i < 5; i++) {
		printf("1");
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
		sleep_ms(100);
		printf("1");
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
		sleep_ms(100);
	}

	printf("Init DAC\n");
	gpio_init(3);
	gpio_set_dir(1, GPIO_OUT);
	gpio_pull_up(3);
	printf("Done\n");

	// SD Card detect testing
	/* gpio_init(26); */
	/* gpio_pull_up(26); */
	/* uint32_t reess; */
	/* while(true) { */
	/* 	printf("Test\n"); */
	/* 	reess = gpio_get(26); */
	/* 	printf("%d\n", reess); */
	/* } */

	printf("Initializing i2c for clock\n");
	i2c_init(i2c0, 100 * 1000);
	gpio_set_function(4, GPIO_FUNC_I2C);
	gpio_set_function(5, GPIO_FUNC_I2C);

	printf("Done. Reading from it\n");
	uint8_t rxdata;
	uint8_t data[] = { 0x0, 0x80 };

	int32_t ret;
	ret = i2c_write_blocking(i2c0, 0x6F, data, sizeof(data), false);
	printf("Done\n");
	printf("%d\n", ret);
	printf("%d\n", PICO_ERROR_GENERIC);

	while(true) {
		sleep_ms(250);
		uint8_t reg = 0x0;
		i2c_write_blocking(i2c0, 0x6F, &reg, 1, true);
		uint8_t value;
		i2c_read_blocking(i2c0, 0x6F, &value, 1, false);
		printf("%d\n", value);
	}

	/* gpio_put(1, 1); */
	/* cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); */
	/* while(true) */

	// Init mag gpios
	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
	for(uint32_t i = 0; i < sizeof(mag_gpios) / sizeof(mag_gpios[0]); i++) {
		gpio_init(mag_gpios[i]);
		gpio_set_dir(i, GPIO_IN);
		printf("Pin initialized\n");
	}

	while(true) {
		selection = 0;
		// Read binary selection from mag gpio
		while (selection == 0) {
			for(uint32_t i = 0; i < sizeof(mag_gpios) / sizeof(mag_gpios[0]); i++) {
				selection <<= 1;
				selection |= gpio_get(mag_gpios[i]);
			}
		}
		printf("%u\n", selection);
	}

	// Mount drive
	printf("Mounting drive\n");
	fr = f_mount(&fs, "0:", 1);
	if (fr != FR_OK) {
		printf("ERROR: Could not mount filesystem (%d)\r\n", fr);
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
		while (true);
	}

	cart_path[0] = '\0';
	// Manually specify a cartridge
	selection = 2;

	// Open cartridge dir to find the first file
	sprintf(cart_path, "cartridge%d/track1", selection);
	printf("Cart path: %s\n", cart_path);
	fr = f_opendir(&cart_dir, cart_path);
	if (fr != FR_OK) {
		printf("ERROR: Could not open directory %s\r\n", cart_path);
	} else {
		printf("Opened up dir: %s\r\n", cart_path);
	}

	fr = f_readdir(&cart_dir, &file_info);
	printf("Filename: %s\n", &file_info.fname);

	// Open file for reading
	printf("Opening file\n");
	sprintf(full_path, "cartridge%d/track1/%s", selection, file_info.fname);
	printf("Full path: %s\n", full_path);
	fr = f_open(&file, full_path, FA_READ);
	if (fr != FR_OK) {
		printf("ERROR: Could not open file (%s)\r\n", fr);
		sleep_ms(3000);
		gpio_put(1, 1);
	}

	// Print every line in file over serial
	printf("Reading from file '%s':\r\n", full_path);
	printf("---\r\n");
	uint32_t res;
	ff_fread(&header, sizeof(WaveHeader), 1, &file);
	printf("%d\n", res);

	printf("RIFF ID: %.4s\n", header.riff_id);
	printf("File size: %d\n", header.filesize);
	printf("File type: %.4s\n", header.filetype);
	printf("Chunk mark: %.4s\n", header.chunk_mark);
	printf("Chunk size: %d\n", header.chunk_size);
	printf("Channels: %d\n", header.channels);

	if (
		strncmp(header.riff_id, "RIFF", 4) == 0 &&
		strncmp(header.filetype, "WAVE", 4) == 0 &&
		header.channels == 2
	) {
		printf("Valid wave file\n");
	} else {
		printf("Not a wave file\n");
	}

	uint8_t bytes_per_sample = header.bits_per_sample / 8;
	for (uint32_t i = 0; i < header.data_size / bytes_per_sample; i++) {
		uint32_t acc_chans = 0;
		for (uint32_t j = 0; j < header.channels; j++) {
			uint32_t sample;
			ff_fread(&sample, bytes_per_sample, 1, &file);
			acc_chans += sample;
		}
		acc_chans /= header.channels;
	}

	// Close file
	printf("Closing file\n");
	fr = f_close(&file);
	if (fr != FR_OK) {
		printf("ERROR: Could not close file (%d)\r\n", fr);
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
		sleep_ms(3000);
		gpio_put(1, 1);
	}

	printf("%d\n", selection);
	sleep_ms(100);

	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
	printf("1");
	sleep_ms(250);
	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
	printf("2");
	sleep_ms(250);

	gpio_put(1, 1);
	while(true);
}
