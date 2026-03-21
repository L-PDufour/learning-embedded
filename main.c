#include "gpio_exti.h"
#include "standby_mode.h"
#include "tim.h"
#include "uart.h"
#include <stdio.h>
#include <string.h>
uint8_t g_btn_press;
void EXTI15_10_IRQHandler(void) {
  if ((EXTI->PR & LINE13) != 0) {
    /*Clear PR flag*/
    EXTI->PR |= LINE13;
    // Do something...
    exti_callback();
  }
}
static void exti_callback(void) { standby_wakeup_pin_setup(); }
static void check_reset_source(void) {
  /*Enable clock access to PWR*/
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
  if ((PWR->CSR & PWR_CSR_SBF) == (PWR_CSR_SBF)) {
    /*Clear Standby flag*/
    PWR->CR |= PWR_CR_CSBF;
    printf("System resume from Standby.....\n\r");
    /*Wait for wakeup pin to be released*/
    while (get_wakeup_pin_state() == 0) {
    }
  }
  /*jjjCheck and Clear Wakeup flag*/
  if ((PWR->CSR & PWR_CSR_WUF) == PWR_CSR_WUF) {
    PWR->CR |= PWR_CR_CWUF;
  }
}
int main(void) {
  tim2_1hz_init();
  while (1) {
  }
}
