#include "tim.h"
#define TIM2EN (1U << 0)

#define GPIOBEN (1U << 1)

void tim2_pwm_init(void) {
  RCC->APB1ENR |= TIM2EN;
  RCC->AHB1ENR |= GPIOBEN;
  GPIOB->AFR[0] |= (1U << 12);
  GPIOB->AFR[0] &= ~(1U << 13);
  GPIOB->AFR[0] &= ~(1U << 14);
  GPIOB->AFR[0] &= ~(1U << 15);
  GPIOB->MODER &= ~(1U << 6);
  GPIOB->MODER |= (1U << 7);
  TIM2->PSC = 0;
  TIM2->ARR = 36363;
  TIM2->CNT = 0;
  TIM2->CCMR1 |= (1U << 14);
  TIM2->CCMR1 |= (1U << 13);
  TIM2->CCMR1 &= ~(1U << 12);
  TIM2->CCMR1 |= (1U << 11);
  TIM2->CCMR1 &= ~(1U << 9);
  TIM2->CCMR1 &= ~(1U << 8);
  TIM2->CCR2 = TIM2->ARR / 2;

  TIM2->EGR |= (1U << 0);
}
