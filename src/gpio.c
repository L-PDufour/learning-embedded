#include "gpio.h"
#include <stddef.h>
#define GPIOAEN (1U << 0)

void button_init(void) {
  RCC->AHB1ENR |= GPIOAEN;

  const uint32_t btn_pins[] = {0, 1, 4, 6, 7, 8, 9, 10};
  for (size_t i = 0; i < sizeof(btn_pins) / sizeof(btn_pins[0]); i++) {
    uint32_t p = btn_pins[i];
    GPIOA->MODER &= ~(3U << (2 * p));
    GPIOA->PUPDR &= ~(3U << (2 * p));
    GPIOA->PUPDR |= (1U << (2 * p));
  }
}
