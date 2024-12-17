#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

int main() {
	stdio_init_all();
	/* cyw43_arch_init(); */

	gpio_init(1);
	gpio_set_dir(1, GPIO_OUT);

	if (cyw43_arch_init()) {
		printf("Wi-Fi init failed");
		return -1;
	}
	for (uint32_t i = 0; i < 5; i++) {
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
		sleep_ms(250);
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
		sleep_ms(250);
	}
	gpio_put(1, 1);
	while(true) {
	}
}
