#include "i2s.h"
#include "platform.h"
#include "stm32f411xe.h"
#include "stm32f4xx.h"
#include <stdint.h>

volatile uint32_t i2s_ht_count = 0;
volatile uint32_t i2s_tc_count = 0;

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
}
void i2s_dma_start(void) { DMA1_Stream7->CR |= DMA_SxCR_EN; }
void DMA1_Stream7_IRQHandler(void) {
  if (DMA1->HISR & DMA_HISR_HTIF7) {
    DMA1->HIFCR = DMA_HIFCR_CHTIF7;
    i2s_ht_count++;
    platform_fill_half(0); /* first half finished: DMA is on the second */
  }
  if (DMA1->HISR & DMA_HISR_TCIF7) {
    DMA1->HIFCR = DMA_HIFCR_CTCIF7;
    i2s_tc_count++;
    platform_fill_half(1); /* all done: DMA wrapped back to the first half */
  }
}