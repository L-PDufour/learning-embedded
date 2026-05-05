#include "engine.h"
#include <stdint.h>
#include <stdio.h>
int main(void) {

  Engine e = engine_init();
  engine_set_bpm(&e, 400);
  engine_set_steps(&e, 32,
                   (MusicNote[]){NOTE_E5, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_C5,
                                 NOTE_B4, NOTE_A4, NOTE_A4, NOTE_C5, NOTE_E5,
                                 NOTE_D5, NOTE_C5, NOTE_B4, NOTE_C5, NOTE_D5,
                                 NOTE_E5, NOTE_C5, NOTE_A4, NOTE_A4, NOTE_REST,
                                 NOTE_D5, NOTE_F5, NOTE_A5, NOTE_G5, NOTE_F5,
                                 NOTE_E5, NOTE_C5, NOTE_E5, NOTE_D5, NOTE_C5,
                                 NOTE_B4, NOTE_B4});
  while (1) {

    int16_t sample = engine_next_sample(&e);
    fwrite(&sample, sizeof(int16_t), 1, stdout);
  }
}

/* #include "gpio.h" */
/* #include "gpio_exti.h" */
/* #include "stm32f4xx.h" */
/* #include "tim.h" */
/* #include <stdint.h> */
/* #define CR1_CEN (1U << 0) */
/* void disable_buzzer(void) { */
/*   TIM2->CR1 &= ~CR1_CEN;    // stop counter */
/*   TIM2->CCER &= ~(1U << 4); // disable CH2 output */
/*   TIM2->CNT = 0;            // reset for clean restart */
/* } */

/* void enable_buzzer(void) { */
/*   TIM2->CNT = 0; */
/*   TIM2->CCER |= (1U << 4); */
/*   TIM2->CR1 |= CR1_CEN; */
/* }; */

/* void noname(uint32_t pin, uint32_t freq) { */
/*   EXTI->PR = (1U << pin);         // clear pending first */
/*   if (GPIOA->IDR & (1U << pin)) { // adjust port/pin to your wiring */
/*     // released (assuming pull-down, active-high) */
/*     disable_buzzer(); */
/*   } else { */
/*     TIM2->ARR = freq; */
/*     TIM2->CCR2 = TIM2->ARR / 2; */
/*     enable_buzzer(); */
/*   } */
/* } */

/* void EXTI0_IRQHandler(void) { noname(0, 30825); } // C4 */
/* void EXTI1_IRQHandler(void) { noname(1, 27259); } // D4 */
/* void EXTI4_IRQHandler(void) { noname(4, 24279); } // E4 */
/* void EXTI9_5_IRQHandler(void) { */
/*   if (EXTI->PR & (1U << 5)) { */
/*     EXTI->PR = (1U << 5); */
/*     noname(5, 22906); */
/*   } // F4 */
/*   if (EXTI->PR & (1U << 6)) { */
/*     EXTI->PR = (1U << 6); */
/*     noname(6, 20408); */
/*   } // G4 */
/*   if (EXTI->PR & (1U << 7)) { */
/*     EXTI->PR = (1U << 7); */
/*     noname(7, 18182); */
/*   } // A4 */
/*   if (EXTI->PR & (1U << 8)) { */
/*     EXTI->PR = (1U << 8); */
/*     noname(8, 16194); */
/*   } // B4 */
/*   if (EXTI->PR & (1U << 9)) { */
/*     EXTI->PR = (1U << 9); */
/*     noname(9, 15289); */
/*   } // C5 */
/* } */
/* void EXTI15_10_IRQHandler(void) { */
/*   if (EXTI->PR & (1U << 10)) { */
/*     EXTI->PR = (1U << 10); */
/*     noname(10, 13621); */
/*   } // D5 */
/* } */
/* int main(void) { */
/*   tim2_pwm_init(); */
/*   button_exti_init(); */
/*   button_init(); */

/*   while (1) { */
/*   } */
/* } */
