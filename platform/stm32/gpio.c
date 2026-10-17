#include "gpio.h"
#include "stm32f4xx.h"
#include "systick.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
  GPIO_TypeDef *port; /* GPIOB or GPIOC */
  uint8_t pin;        /* 0..15 */
} PadPin;

static const PadPin pad_map[PADS_MAX] = {
    {GPIOB, 7},  {GPIOB, 8},  {GPIOB, 10}, {GPIOB, 12},
    {GPIOB, 13}, {GPIOB, 14}, {GPIOB, 15}, {GPIOC, 0},
};

void pad_init(void) {
  size_t i;

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

  for (i = 0; i < PADS_MAX; i++) {
    uint32_t p = pad_map[i].pin;
    GPIO_TypeDef *port = pad_map[i].port;

    port->MODER &= ~(3U << (2 * p)); /* 00 = input        */
    port->PUPDR &= ~(3U << (2 * p)); /* clear both bits   */
    port->PUPDR |= (1U << (2 * p));  /* 01 = pull-up      */
  }
}

uint8_t pad_read(void) {
  uint8_t mask = 0;
  size_t i;

  for (i = 0; i < PADS_MAX; i++) {
    GPIO_TypeDef *port = pad_map[i].port;
    uint32_t p = pad_map[i].pin;

    if (((port->IDR >> p) & 1U) == 0U) { /* active-low: 0 = pressed */
      mask |= (uint8_t)(1U << i);
    }
  }
  return mask;
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

void led_on(void) { GPIOD->BSRR = GPIO_BSRR_BS13; }

void led_off(void) { GPIOD->BSRR = GPIO_BSRR_BR13; }
