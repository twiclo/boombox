#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"

int main() {
	stdio_init_all();

	// LED on pico
	const uint32_t led_pin = 25;
	const uint32_t btn_pin = 14;
	const uint32_t btn2_pin = 15;
	const uint32_t btn3_pin = 0;

	uint32_t selection = 0;
	uint32_t cartridge[] = {2, 3, 4, 5, 6};
	for(uint32_t i = 0; i < sizeof(cartridge) / sizeof(cartridge[0]); i++) {
		gpio_init(cartridge[i]);
		gpio_set_dir(i, GPIO_IN);
	}

	// Write a function to detect the reason for waking
	while (true) {
		for(uint32_t i = 0; i < sizeof(cartridge) / sizeof(cartridge[0]); i++) {
			selection <<= 1;
			selection |= gpio_get(cartridge[i]);
		}
		printf("%d\r\n", selection);

		if (selection == 0) {
			// Booted for some reason here. Not a button press
			break;
		} else {
			// Booted by a button press
			break;
		}
		sleep_ms(100);
	}
} 
