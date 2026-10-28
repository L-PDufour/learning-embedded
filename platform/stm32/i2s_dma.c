#include "i2s.h"
#include "platform.h"
#include "stm32f411xe.h"
#include "stm32f4xx.h"
#include <stdint.h>

volatile uint32_t i2s_ht_count = 0;
volatile uint32_t i2s_tc_count = 0;
/* Counts entries that found both flags pending, i.e. the refill lost at least
 * one half period. Kept as instrumentation: it should stay near zero. */
volatile uint32_t i2s_late_count = 0;


void i2s_dma_config(sample_t *buf, uint32_t len) {
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
  SPI3->CR2 |= SPI_CR2_TXDMAEN;
  DMA1->HIFCR = DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CHTIF7 |
                DMA_HIFCR_CTCIF7 | DMA_HIFCR_CFEIF7;

  DMA1_Stream7->CR &= ~DMA_SxCR_EN;
  while (DMA1_Stream7->CR & DMA_SxCR_EN) {
  }

  DMA1_Stream7->PAR = (uint32_t)&SPI3->DR;
  DMA1_Stream7->M0AR = (uint32_t)buf;
  DMA1_Stream7->NDTR = (uint16_t)len;
  DMA1_Stream7->FCR = 0;

  DMA1_Stream7->CR = (2U << DMA_SxCR_PL_Pos) | DMA_SxCR_MSIZE_0 |
                     DMA_SxCR_PSIZE_0 | DMA_SxCR_MINC | DMA_SxCR_CIRC |
                     DMA_SxCR_DIR_0 | DMA_SxCR_HTIE | DMA_SxCR_TCIE;
  NVIC_EnableIRQ(DMA1_Stream7_IRQn);
  NVIC_SetPriority(DMA1_Stream7_IRQn, 1);
}
void i2s_dma_start(void) { DMA1_Stream7->CR |= DMA_SxCR_EN; }

void DMA1_Stream7_IRQHandler(void) {
  uint32_t pending = DMA1->HISR & (DMA_HISR_HTIF7 | DMA_HISR_TCIF7);

  if (pending == 0) {
    return;
  }

  if (pending & DMA_HISR_HTIF7) {
    i2s_ht_count++;
  }
  if (pending & DMA_HISR_TCIF7) {
    i2s_tc_count++;
  }
  DMA1->HIFCR = DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTCIF7;

  /* Choose the half the DMA is NOT reading, from the flag that fired - never
   * from NDTR, which can still hold 0 at the transfer-complete interrupt and
   * so "looks" like the first half.
   *
   *   HT only : half 0 finished, the DMA is reading half 1  -> fill 0
   *   TC only : half 1 finished, the DMA wrapped to half 0  -> fill 1
   *   both    : the DMA passed both, so it is reading half 0 again -> fill 1
   *
   * Exactly one fill per entry, always. The old code did one fill per pending
   * flag, so a late entry ran two (~6 ms against a 5.33 ms half), fell a whole
   * half behind, and kept overwriting the half being played. */
  if (pending == (DMA_HISR_HTIF7 | DMA_HISR_TCIF7)) {
    i2s_late_count++; /* we lost at least one half period */
    platform_fill_half(1);
  } else if (pending & DMA_HISR_HTIF7) {
    platform_fill_half(0);
  } else {
    platform_fill_half(1);
  }
}
