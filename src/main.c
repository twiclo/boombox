#include <string.h>
#include "ff.h" /* Obtains integer types */
#include "ff_stdio.h" /* Obtains integer types */
#include "diskio.h" /* Declarations of disk functions */
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/clocks.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "audio_i2s.pio.h"
#include <limits.h>

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
} WaveHeader;

typedef struct __attribute__((packed)) {
	char chunk_id[4];
	uint32_t chunk_size;
} ChunkHeader;

const static uint32_t SPKR_EN = 3;
const static uint32_t I2S_BCLOCK = 20;
const static uint32_t I2S_LRCLK = 21;
const static uint32_t I2S_DATA = 22;

const static uint32_t AUX_EN = 28;

int main() {
	FRESULT fr;
	FATFS fs;
	FIL file;
	char full_path[50];

	char cart_path[12];
	DIR cart_dir;
	FILINFO file_info;
	WaveHeader wave_header;
	ChunkHeader chunk_header;


	uint32_t selection = 0;
	uint32_t mag_gpios[] = {6, 7, 8, 9, 10};

	/* cyw43_arch_init(); */
	stdio_init_all();

	/* printf("\r\nPress 'enter' to start.\r\n"); */
	/* while (true) { */
	/* 	char buf[2]; */
	/* 	buf[0] = getchar(); */
	/* 	if ((buf[0] == '\r') || (buf[0] == '\n')) { */
	/* 		break; */
	/* 	} */
	/* } */

	// Init power off gpio
	gpio_init(1);
	gpio_set_dir(1, GPIO_OUT);

	/* for (uint32_t i = 0; i < 5; i++) { */
	/* 	printf("1"); */
	/* 	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); */
	/* 	sleep_ms(100); */
	/* 	printf("1"); */
	/* 	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0); */
	/* 	sleep_ms(100); */
	/* } */

	printf("Init AUX\n");
	gpio_init(AUX_EN);
	gpio_set_dir(AUX_EN, GPIO_OUT);
	gpio_put(AUX_EN, 0);

	printf("Init SPK\n");
	gpio_init(SPKR_EN);
	gpio_set_dir(SPKR_EN, GPIO_OUT);
	gpio_put(SPKR_EN, 1);

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

	uint8_t rxdata;
	uint8_t data[] = { 0x0, 0x80 };

	int32_t ret;
	ret = i2c_write_blocking(i2c0, 0x6F, data, sizeof(data), false);
	printf("Done\n");
	printf("%d\n", ret);
	printf("%d\n", PICO_ERROR_GENERIC);

	/* while(true) { */
	/* 	sleep_ms(250); */
	/* 	uint8_t reg = 0x0; */
	/* 	i2c_write_blocking(i2c0, 0x6F, &reg, 1, true); */
	/* 	uint8_t value; */
	/* 	i2c_read_blocking(i2c0, 0x6F, &value, 1, false); */
	/* 	printf("%d\n", value); */
	/* } */

	/* gpio_put(1, 1); */
	/* cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); */
	/* while(true) */


	// Init mag gpios
	//cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
	for(uint32_t i = 0; i < sizeof(mag_gpios) / sizeof(mag_gpios[0]); i++) {
		gpio_init(mag_gpios[i]);
		gpio_set_dir(mag_gpios[i], GPIO_IN);
		printf("Pin initialized\n");
	}

	/* while(true) { */
	/* 	selection = 0; */
	/* 	// Read binary selection from mag gpio */
	/* 	while (selection == 0) { */
	/* 		for(uint32_t i = 0; i < sizeof(mag_gpios) / sizeof(mag_gpios[0]); i++) { */
	/* 			selection <<= 1; */
	/* 			selection |= gpio_get(mag_gpios[i]); */
	/* 		} */
	/* 	} */
	/* 	printf("%u\n", selection); */
	/* } */

	// Mount drive
	printf("Mounting drive\n");
	fr = f_mount(&fs, "0:", 1);
	if (fr != FR_OK) {
		printf("ERROR: Could not mount filesystem (%d)\r\n", fr);
		/* cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); */
		while (true);
	}

	cart_path[0] = '\0';
	// Manually specify a cartridge
	selection = 3;

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
	ff_fread(&wave_header, sizeof(WaveHeader), 1, &file);

	printf("RIFF ID: %.4s\n", wave_header.riff_id);
	printf("File size: %d\n", wave_header.filesize);
	printf("File type: %.4s\n", wave_header.filetype);
	printf("Chunk mark: %.4s\n", wave_header.chunk_mark);
	printf("Chunk size: %d\n", wave_header.chunk_size);
	printf("Channels: %d\n", wave_header.channels);
	printf("Sample Rate: %d\n", wave_header.sample_rate);

	if (
		!strncmp(wave_header.riff_id, "RIFF", 4) == 0 &&
		!strncmp(wave_header.filetype, "WAVE", 4) == 0
	) {
		printf("Not a wave file\n");
		gpio_put(1, 1);
		while(true);
	}

	while (ff_fread(&chunk_header, sizeof(ChunkHeader), 1, &file) == 1) {
		if (strncmp(chunk_header.chunk_id, "data", 4) == 0) {
			printf("Found data chunk. Size: %u bytes\n", chunk_header.chunk_size);
		}
		break;
		ff_fseek(&file, chunk_header.chunk_size, SEEK_CUR);
	}
	printf("Exited header loop");

	PIO pio;
	uint sm;
	uint offset;

	bool init_sm = pio_claim_free_sm_and_add_program(&audio_i2s_program, &pio, &sm, &offset);
	if (!init_sm) {
		printf("Failed to get sm\n");
		gpio_put(1, 1);
		while(true);
	}

	// Test clock
	pio_gpio_init(pio, I2S_BCLOCK);
	pio_gpio_init(pio, I2S_LRCLK);
	pio_gpio_init(pio, I2S_DATA);
	audio_i2s_program_init(pio, sm, offset, I2S_DATA, I2S_BCLOCK);
	pio_sm_set_enabled(pio, sm, true);
	printf("Done\n");

	float divider = clock_get_hz(clk_sys) / (float)(wave_header.sample_rate * 2 * 32 * 2);
	printf("divider: %f\n", divider);
	pio_sm_set_clkdiv(pio, sm, divider);

	uint8_t bytes_per_sample = wave_header.bits_per_sample / 8;
	printf("bytes_per_sample: %d\n", bytes_per_sample);

	int dma_chan = dma_claim_unused_channel(false);
	if (!dma_chan == -1) {
		printf("Failed to get dma channel\n");
		gpio_put(1, 1);
		while(true);
	}

	static const uint32_t CHUNK_SIZE = 4096;
	dma_channel_config c = dma_channel_get_default_config(dma_chan);
	channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
	channel_config_set_read_increment(&c, true);
	channel_config_set_write_increment(&c, false);
	channel_config_set_dreq(&c, pio_get_dreq(pio, sm, true));
	dma_channel_configure(dma_chan, &c, &pio->txf[sm], NULL, CHUNK_SIZE, false);

	int32_t buf1[CHUNK_SIZE];
	int32_t buf2[CHUNK_SIZE];
	int32_t* cur_buf = buf1;
	for (uint32_t i = 0; i < chunk_header.chunk_size / bytes_per_sample / CHUNK_SIZE; i++) {
		for (uint32_t j = 0; j < CHUNK_SIZE; j = j + 2) {
			int32_t sample;
			int32_t acc_chans = 0;
			for (uint32_t k = 0; k < wave_header.channels; k++) {
				uint8_t bytes[4];
				ff_fread(bytes, bytes_per_sample, 1, &file);
				switch(bytes_per_sample) {
					case 1:
						sample = bytes[0];
						break;
					case 2:
						sample = *(int16_t*)bytes;
						break;
					case 4:
						sample = *(int32_t*)bytes;
						break;
					default:
						gpio_put(1, 1);
						while(true);
						break;
				}
				acc_chans += sample;
			}
			sample = acc_chans / wave_header.channels;

			int8_t sign = sample < 0 ? -1 : 1;
			sample = (abs(sample) << (32 - wave_header.bits_per_sample)) | abs(sample);
			sample *= sign;

			cur_buf[j] = sample / 24;
			cur_buf[j + 1] = sample / 24;
		}
		dma_channel_wait_for_finish_blocking(dma_chan);
		dma_channel_set_read_addr(dma_chan, cur_buf, true);

		// Fancyify this later
		if (cur_buf == buf1) {
			cur_buf = buf2;
		} else {
			cur_buf = buf1;
		}
	}
	/* cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0); */
	gpio_put(1, 1);
	while(true) {
		printf("1");
	};
	dma_channel_wait_for_finish_blocking(dma_chan);

	// Close file
	printf("Closing file\n");
	fr = f_close(&file);
	if (fr != FR_OK) {
		printf("ERROR: Could not close file (%d)\r\n", fr);
		/* cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); */
		sleep_ms(3000);
		gpio_put(1, 1);
	}

	/* printf("%d\n", selection); */
	/* sleep_ms(100); */

	gpio_put(1, 1);
	while(true);
}
