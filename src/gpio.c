#include "gpio.h"
#define GPIOAEN (1U << 0)
#define GPIOBEN (1U << 1)
#define LED_BS5 (1U << 5)  /*Bit Set Pin 5*/
#define LED_BR5 (1U << 21) /*Bit Reset Pin 5*/
#define GPIOAEN (1U << 0)
#define GPIOCEN (1U << 2)
#define BTN_PIN (1U << 13)
#define LED_PIN (1U << 5)

void button_init(void) {

  RCC->AHB1ENR |= GPIOAEN;

  GPIOA->MODER |= ~(1U << 0);
  GPIOA->MODER &= ~(1U << 1);

  GPIOA->MODER |= ~(1U << 2);
  GPIOA->MODER &= ~(1U << 3);

  GPIOA->MODER |= ~(1U << 8);
  GPIOA->MODER &= ~(1U << 9);

  GPIOA->MODER |= ~(1U << 12);
  GPIOA->MODER &= ~(1U << 13);

  GPIOA->MODER |= ~(1U << 14);
  GPIOA->MODER &= ~(1U << 15);

  GPIOA->MODER |= ~(1U << 16);
  GPIOA->MODER &= ~(1U << 17);

  GPIOA->MODER |= ~(1U << 18);
  GPIOA->MODER &= ~(1U << 19);

  GPIOA->MODER |= ~(1U << 20);
  GPIOA->MODER &= ~(1U << 21);

  GPIOA->PUPDR |= ~(1U << 0);
  GPIOA->PUPDR &= ~(1U << 1);

  GPIOA->PUPDR |= ~(1U << 2);
  GPIOA->PUPDR &= ~(1U << 3);

  GPIOA->PUPDR |= ~(1U << 8);
  GPIOA->PUPDR &= ~(1U << 9);

  GPIOA->PUPDR |= ~(1U << 12);
  GPIOA->PUPDR &= ~(1U << 13);

  GPIOA->PUPDR |= ~(1U << 14);
  GPIOA->PUPDR &= ~(1U << 15);

  GPIOA->PUPDR |= ~(1U << 16);
  GPIOA->PUPDR &= ~(1U << 17);

  GPIOA->PUPDR |= ~(1U << 18);
  GPIOA->PUPDR &= ~(1U << 19);

  GPIOA->PUPDR |= ~(1U << 20);
  GPIOA->PUPDR &= ~(1U << 21);
}

bool get_btn_state(void) {
  if (GPIOC->IDR & BTN_PIN) {
    return false;
  } else {
    return true;
  }
}
void led_init(void) {
  /*Enable clock access to GPIOA*/
  RCC->AHB1ENR |= GPIOAEN;
  /*Set PA5 mode to output mode*/
  GPIOA->MODER |= (1U << 10);
  GPIOA->MODER &= ~(1U << 11);
}
void led_on(void) {
  /*Set PA5 high*/
  GPIOA->BSRR |= LED_BS5;
}
void led_off(void) {
  /*Set PA5 high*/
  GPIOA->BSRR |= LED_BR5;
}

void led_toggle(void) {
  /*Toggle PA5*/
  GPIOA->ODR ^= LED_PIN;
}

void alternate_function_init(void) {
  RCC->AHB1ENR |= GPIOBEN;
  GPIOB->AFR[0] |= (1U << 12);
  GPIOB->AFR[0] &= ~(1U << 13);
  GPIOB->AFR[0] &= ~(1U << 14);
  GPIOB->AFR[0] &= ~(1U << 15);
  GPIOB->MODER &= ~(1U << 6);
  GPIOB->MODER |= (1U << 7);
}
