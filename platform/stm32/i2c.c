#include "i2c.h"
#include "stm32f4xx.h"
#include "systick.h"
#include <stdint.h>

#define GPIOBEN (1U << 1)
#define I2C1EN (1U << 21)
#define I2C_100KHZ 80
#define SD_MODE_MAX_RISE_TIME 17
#define CR1_PE (1U << 0)
#define SR2_BUSY (1U << 1)
#define CR1_START (1U << 8)
#define SR1_SB (1U << 0)
#define SR1_ADDR (1U << 1)
#define SR1_TXE (1U << 7)
#define SR1_AF (1U << 10)
#define CR1_ACK (1U << 10)
#define CR1_STOP (1U << 9)
#define SR1_RXNE (1U << 6)
#define SR1_BTF (1U << 2)

/* Bound for any single flag wait, in milliseconds. The happy path exits in
 * microseconds; this only bounds genuine failures (NACK / stuck bus). */
#define I2C_TIMEOUT_MS 10

/*
 * PB6 ---- SCL
 * PB9 ----- SDA
 * */

static void set_alt_fn(int pin) {
  GPIOB->MODER &= ~(3U << (2 * pin));
  GPIOB->MODER |= (2U << (2 * pin));
}

static void set_open_drain(int pin) { GPIOB->OTYPER |= (1U << pin); }

static void set_afr_alt_i2c(int pin) {
  GPIOB->AFR[pin >> 3] &= ~(0xFU << ((pin & 7) * 4));
  GPIOB->AFR[pin >> 3] |= (4U << ((pin & 7) * 4));
}

void i2c1_init(void) {
  /*Enable clock access to GPIOB*/
  RCC->AHB1ENR |= GPIOBEN;

  set_alt_fn(6);
  set_alt_fn(9);

  set_open_drain(6);
  set_open_drain(9);

  set_afr_alt_i2c(6);
  set_afr_alt_i2c(9);

  /*Enable clock access to I2C1*/
  RCC->APB1ENR |= I2C1EN;
  /*Enter reset mode  */
  I2C1->CR1 |= (1U << 15);
  /*Come out of reset mode  */
  I2C1->CR1 &= ~(1U << 15);
  /*Set Peripheral clock frequency: FREQ = PCLK1 in MHz (16 MHz)*/
  I2C1->CR2 = 16;
  /*Set I2C to standard mode, 100kHz clock */
  I2C1->CCR = I2C_100KHZ;
  /*Set rise time */
  I2C1->TRISE = SD_MODE_MAX_RISE_TIME;
  /*Enable I2C1 module */
  I2C1->CR1 |= CR1_PE;
}

I2cStatus i2c1_byte_read(uint8_t saddr, uint8_t maddr, uint8_t *data) {
  uint32_t deadline;

  /* Wait until bus not busy */
  deadline = systick_millis() + I2C_TIMEOUT_MS;
  while (I2C1->SR2 & SR2_BUSY) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }

  /* Generate start */
  I2C1->CR1 |= CR1_START;
  deadline = systick_millis() + I2C_TIMEOUT_MS;
  while (!(I2C1->SR1 & SR1_SB)) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }

  /* Transmit slave address + Write */
  I2C1->DR = saddr << 1;

  /* Wait until address is ACKed, or NACK */
  deadline = systick_millis() + I2C_TIMEOUT_MS;
  while (!(I2C1->SR1 & (SR1_ADDR | SR1_AF))) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }
  if (I2C1->SR1 & SR1_AF) { /* NACK: no device answered */
    I2C1->SR1 &= ~SR1_AF;   /* clear failure (write 0 to AF) */
    I2C1->CR1 |= CR1_STOP;  /* release the bus */
    return I2C_ERR_NACK;
  }

  /* Clear addr flag */
  (void)I2C1->SR2; /* read SR2 to clear the ADDR flag */

  /* Send memory address */
  I2C1->DR = maddr;
  deadline = systick_millis() + I2C_TIMEOUT_MS;
  while (!(I2C1->SR1 & SR1_TXE)) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }

  /*Generate restart */
  I2C1->CR1 |= CR1_START;
  deadline = systick_millis() + I2C_TIMEOUT_MS;
  while (!(I2C1->SR1 & SR1_SB)) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }

  /* Transmit slave address + Read */
  I2C1->DR = (saddr << 1) | 1;

  /* Wait until address is ACKed, or NACK */
  deadline = systick_millis() + I2C_TIMEOUT_MS;
  while (!(I2C1->SR1 & (SR1_ADDR | SR1_AF))) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }
  if (I2C1->SR1 & SR1_AF) { /* NACK: no device answered */
    I2C1->SR1 &= ~SR1_AF;
    I2C1->CR1 |= CR1_STOP;
    return I2C_ERR_NACK;
  }

  /* Disable Acknowledge (single byte) */
  I2C1->CR1 &= ~CR1_ACK;
  /* Clear addr flag */
  (void)I2C1->SR2; /* read SR2 to clear the ADDR flag */
  /* Generate stop after data received */
  I2C1->CR1 |= CR1_STOP;

  /* Wait until RXNE flag is set */
  deadline = systick_millis() + I2C_TIMEOUT_MS;
  while (!(I2C1->SR1 & SR1_RXNE)) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }

  /* Read data from DR */
  *data = (uint8_t)I2C1->DR;
  return I2C_OK;
}

I2cStatus i2c1_burst_read(uint8_t saddr, uint8_t maddr, int n, uint8_t *data) {
  uint32_t deadline;

  /* Wait until bus not busy */
  deadline = systick_millis() + I2C_TIMEOUT_MS;
  while (I2C1->SR2 & SR2_BUSY) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }

  /* Generate start */
  I2C1->CR1 |= CR1_START;
  deadline = systick_millis() + I2C_TIMEOUT_MS;
  while (!(I2C1->SR1 & SR1_SB)) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }

  /* Transmit slave address + Write */
  I2C1->DR = saddr << 1;
  deadline = systick_millis() + I2C_TIMEOUT_MS;
  while (!(I2C1->SR1 & (SR1_ADDR | SR1_AF))) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }
  if (I2C1->SR1 & SR1_AF) {
    I2C1->SR1 &= ~SR1_AF;
    I2C1->CR1 |= CR1_STOP;
    return I2C_ERR_NACK;
  }

  /* Clear addr flag */
  (void)I2C1->SR2; /* read SR2 to clear the ADDR flag */

  /* Send memory address */
  deadline = systick_millis() + I2C_TIMEOUT_MS;
  while (!(I2C1->SR1 & SR1_TXE)) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }
  I2C1->DR = maddr;
  deadline = systick_millis() + I2C_TIMEOUT_MS;
  while (!(I2C1->SR1 & SR1_TXE)) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }

  /*Generate restart */
  I2C1->CR1 |= CR1_START;
  deadline = systick_millis() + I2C_TIMEOUT_MS;
  while (!(I2C1->SR1 & SR1_SB)) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }

  /* Transmit slave address + Read */
  I2C1->DR = (saddr << 1) | 1;
  deadline = systick_millis() + I2C_TIMEOUT_MS;
  while (!(I2C1->SR1 & (SR1_ADDR | SR1_AF))) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }
  if (I2C1->SR1 & SR1_AF) {
    I2C1->SR1 &= ~SR1_AF;
    I2C1->CR1 |= CR1_STOP;
    return I2C_ERR_NACK;
  }

  /* Clear addr flag */
  (void)I2C1->SR2; /* read SR2 to clear the ADDR flag */
  /* Enable Acknowledge */
  I2C1->CR1 |= CR1_ACK;
  while (n > 0) {
    /*if one byte*/
    if (n == 1) {
      /* Disable Acknowledge */
      I2C1->CR1 &= ~CR1_ACK;
      /* Generate Stop */
      I2C1->CR1 |= CR1_STOP;
      /* Wait for RXNE flag set */
      deadline = systick_millis() + I2C_TIMEOUT_MS;
      while (!(I2C1->SR1 & SR1_RXNE)) {
        if ((int32_t)(systick_millis() - deadline) >= 0) {
          return I2C_ERR_TIMEOUT;
        }
      }
      /* Read data from DR */
      *data = (uint8_t)I2C1->DR;
      break;
    } else {
      /* Wait until RXNE flag is set */
      deadline = systick_millis() + I2C_TIMEOUT_MS;
      while (!(I2C1->SR1 & SR1_RXNE)) {
        if ((int32_t)(systick_millis() - deadline) >= 0) {
          return I2C_ERR_TIMEOUT;
        }
      }
      /* Read data from DR */
      *data = (uint8_t)I2C1->DR;
      data++;
      n--;
    }
  }
  return I2C_OK;
}

I2cStatus i2c1_burst_write(uint8_t saddr, uint8_t maddr, int n,
                           const uint8_t *data) {
  uint32_t deadline;
  int i;

  /* Wait until bus not busy */
  deadline = systick_millis() + I2C_TIMEOUT_MS;
  while (I2C1->SR2 & SR2_BUSY) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }

  /* Generate start */
  I2C1->CR1 |= CR1_START;
  deadline = systick_millis() + I2C_TIMEOUT_MS;
  while (!(I2C1->SR1 & SR1_SB)) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }

  /* Transmit slave address + Write */
  I2C1->DR = saddr << 1;
  deadline = systick_millis() + I2C_TIMEOUT_MS;
  while (!(I2C1->SR1 & (SR1_ADDR | SR1_AF))) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }
  if (I2C1->SR1 & SR1_AF) {
    I2C1->SR1 &= ~SR1_AF;
    I2C1->CR1 |= CR1_STOP;
    return I2C_ERR_NACK;
  }

  /* Clear addr flag */
  (void)I2C1->SR2; /* read SR2 to clear the ADDR flag */

  /* Wait until data register empty */
  deadline = systick_millis() + I2C_TIMEOUT_MS;
  while (!(I2C1->SR1 & SR1_TXE)) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }
  /* Send memory address */
  I2C1->DR = maddr;
  for (i = 0; i < n; i++) {
    /* Wait until data register empty */
    deadline = systick_millis() + I2C_TIMEOUT_MS;
    while (!(I2C1->SR1 & SR1_TXE)) {
      if ((int32_t)(systick_millis() - deadline) >= 0) {
        return I2C_ERR_TIMEOUT;
      }
    }
    /* Transmit data byte */
    I2C1->DR = data[i];
  }
  /* Wait until transfer finished */
  deadline = systick_millis() + I2C_TIMEOUT_MS;
  while (!(I2C1->SR1 & SR1_BTF)) {
    if ((int32_t)(systick_millis() - deadline) >= 0) {
      return I2C_ERR_TIMEOUT;
    }
  }
  /* Generate stop */
  I2C1->CR1 |= CR1_STOP;
  return I2C_OK;
}

I2cStatus i2c1_byte_write(uint8_t saddr, uint8_t maddr, uint8_t data) {
  return i2c1_burst_write(saddr, maddr, 1, &data);
}
