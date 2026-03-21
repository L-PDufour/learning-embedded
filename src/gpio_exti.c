#include "gpio_exti.h"
#define GPIOAEN (1U << 0)
#define SYSCFGEN (1U << 14)

void button_exti_init(void) {
  __disable_irq();

  RCC->AHB1ENR |= GPIOAEN;

  RCC->APB2ENR |= SYSCFGEN;
  EXTI->IMR |= (1U << 0);
  EXTI->IMR |= (1U << 1);
  EXTI->IMR |= (1U << 4);
  EXTI->IMR |= (1U << 6);
  EXTI->IMR |= (1U << 7);
  EXTI->IMR |= (1U << 8);
  EXTI->IMR |= (1U << 9);
  EXTI->IMR |= (1U << 10);

  EXTI->FTSR |= (1U << 0);
  EXTI->FTSR |= (1U << 1);
  EXTI->FTSR |= (1U << 4);
  EXTI->FTSR |= (1U << 6);
  EXTI->FTSR |= (1U << 7);
  EXTI->FTSR |= (1U << 8);
  EXTI->FTSR |= (1U << 9);
  EXTI->FTSR |= (1U << 10);

  NVIC_EnableIRQ(EXTI0_IRQn);
  NVIC_EnableIRQ(EXTI1_IRQn);
  NVIC_EnableIRQ(EXTI4_IRQn);
  NVIC_EnableIRQ(EXTI9_5_IRQn);
  NVIC_EnableIRQ(EXTI15_10_IRQn);

  __enable_irq();
}
