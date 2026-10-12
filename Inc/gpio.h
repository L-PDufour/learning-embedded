#ifndef GPIO_H_
#define GPIO_H_

#include <stdint.h>

#define PADS_MAX 8

void pad_init(void);
uint8_t pad_read(void);

void led_init(void);
void board_init(void);
void led_on(void);
void led_off(void);
void led_toggle(void);

#endif
