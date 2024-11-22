#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

int main() {
	stdio_init_all();
	/* cyw43_arch_init(); */

	if (cyw43_arch_init()) {
		printf("Wi-Fi init failed");
		return -1;
	}
	while (true) {
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
		sleep_ms(250);
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
		sleep_ms(250);
	}
}

/* #include <stdio.h> */
/* #include <stdint.h> */
/* #include <stdbool.h> */
/* #include "pico/stdlib.h" */

/* int main() { */
/* 	stdio_init_all(); */

/* 	// LED on pico */
/* 	const uint32_t led_pin = 25; */
/* 	const uint32_t btn_pin = 14; */
/* 	const uint32_t btn2_pin = 15; */
/* 	const uint32_t btn3_pin = 0; */

/* 	uint32_t selection = 0; */
/* 	uint32_t cartridge[] = {2, 3, 4, 5, 6}; */
/* 	for(uint32_t i = 0; i < sizeof(cartridge) / sizeof(cartridge[0]); i++) { */
/* 		gpio_init(cartridge[i]); */
/* 		gpio_set_dir(i, GPIO_IN); */
/* 	} */

/* 	/1* Write a function to detect the reason for waking *1/ */
/* 	while (true) { */
/* 		for(uint32_t i = 0; i < sizeof(cartridge) / sizeof(cartridge[0]); i++) { */
/* 			selection <<= 1; */
/* 			selection |= gpio_get(cartridge[i]); */
/* 		} */
/* 		printf("%d\r\n", selection); */

/* 		if (selection == 0) { */
/* 			break; */
/* 		} else { */
/* 			/1* play_boot(); *1/ */
/* 			break; */
/* 		} */

/* 		if (!gpio_get(btn_pin)) { */
/* 			gpio_put(led_pin, 1); */
/* 			printf("Button 1\r\n"); */
/* 		} else if (!gpio_get(btn2_pin)) { */
/* 			printf("Button 2\r\n"); */
/* 		} else if (!gpio_get(btn3_pin)) { */
/* 			printf("Button 3\r\n"); */
/* 		} else { */
/* 			gpio_put(led_pin, 0); */
/* 		} */
/* 		sleep_ms(100); */
/* 	} */
/* } */ 

/* #include <stdio.h> */
/* #include <stdint.h> */
/* #include <stdbool.h> */
/* #include "pico/stdlib.h" */

/* int main() { */
/* 	stdio_init_all(); */

/* 	const uint32_t led_pin = 25; */
/* 	gpio_init(led_pin); */
/* 	gpio_set_dir(led_pin, GPIO_OUT); */

/* 	while (true) { */
/* 		gpio_put(led_pin, 1); */
/* 		sleep_ms(500); */
/* 		gpio_put(led_pin, 0); */
/* 		sleep_ms(500); */
/* 	} */
/* } */
