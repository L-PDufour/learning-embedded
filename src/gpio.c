#include "gpio.h"
#include "systick.h"
#include <stddef.h>

void button_init(void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  const uint32_t btn_pins[] = {0, 1, 4, 6, 7, 8, 9, 10};
  for (size_t i = 0; i < sizeof(btn_pins) / sizeof(btn_pins[0]); i++) {
    uint32_t p = btn_pins[i];
    GPIOA->MODER &= ~(3U << (2 * p));
    GPIOA->PUPDR &= ~(3U << (2 * p));
    GPIOA->PUPDR |= (1U << (2 * p));
  }
}

static void codec_reset(void) {
  GPIOD->BSRR = (1U << (4 + 16)); /* reset: drive PD4 low  (or GPIO_BSRR_BR4) */
  systick_msec_delay(1);
  GPIOD->BSRR = GPIO_BSRR_BS4; /* release: drive PD4 high */
  systick_msec_delay(1);
}

void board_init(void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
  GPIOD->MODER &= ~GPIO_MODER_MODER4_Msk;
  GPIOD->MODER |= GPIO_MODER_MODER4_0;
  GPIOD->OTYPER &= ~GPIO_OTYPER_OT4;
  codec_reset();
}

void led_init(void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
  GPIOD->MODER &= ~GPIO_MODER_MODER13_Msk;
  GPIOD->MODER |= GPIO_MODER_MODER13_0;
}

void led_toggle(void) { GPIOD->ODR ^= (1U << 13); }
