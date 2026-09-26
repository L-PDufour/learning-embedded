#include "i2s.h"
#include "i2c.h"
#include "stm32f411xe.h"
#include "stm32f4xx.h"
#include "systick.h"

#define CLOCK_TIMEOUT_MS 100

static void set_alt_fn(int pin) {
  GPIOC->MODER &= ~(3U << (2 * pin));
  GPIOC->MODER |= (2U << (2 * pin));
}

I2cStatus i2s_clock_init(void) {
  uint32_t deadline;
  RCC->CR |= RCC_CR_HSEON;
  deadline = systick_millis() + CLOCK_TIMEOUT_MS;
  while (!(RCC->CR & RCC_CR_HSERDY)) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }
  RCC->PLLI2SCFGR = (8U << RCC_PLLI2SCFGR_PLLI2SM_Pos) |
                    (258U << RCC_PLLI2SCFGR_PLLI2SN_Pos) |
                    (3U << RCC_PLLI2SCFGR_PLLI2SR_Pos);
  RCC->CR |= RCC_CR_PLLI2SON;
  deadline = systick_millis() + CLOCK_TIMEOUT_MS;
  while (!(RCC->CR & RCC_CR_PLLI2SRDY)) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }
  return I2C_OK;
}

I2cStatus i2s3_init(void) {
  RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN);
  RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;

  GPIOA->MODER &= ~(3U << (2 * 4));
  GPIOA->MODER |= (2U << (2 * 4));
  set_alt_fn(7);
  set_alt_fn(10);
  set_alt_fn(12);

  GPIOA->AFR[0] &= ~(0xFU << 16);
  GPIOA->AFR[0] |= (6U << 16);
  GPIOC->AFR[0] &= ~(0xFU << 28);
  GPIOC->AFR[0] |= (6U << 28);
  GPIOC->AFR[1] &= ~(0xFU << 8);
  GPIOC->AFR[1] |= (6U << 8);
  GPIOC->AFR[1] &= ~(0xFU << 16);
  GPIOC->AFR[1] |= (6U << 16);
  SPI3->I2SCFGR |= (SPI_I2SCFGR_I2SMOD | SPI_I2SCFGR_I2SCFG_1);
  SPI3->I2SPR |= (3 | SPI_I2SPR_ODD | SPI_I2SPR_MCKOE);
  SPI3->I2SCFGR |= SPI_I2SCFGR_I2SE;
  return I2C_OK;
}
