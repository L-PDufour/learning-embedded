#include "gpio.h"
#include "platform.h"
#include "systick.h"

void platform_init(void) {
  led_init();
  board_init();
  while (1) {
    systick_msec_delay(1000);
    led_toggle();
  }
}
