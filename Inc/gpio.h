#ifndef GPIO_H_
#define GPIO_H_

#include <stdint.h>

#define PADS_MAX 8
#define BUTTONS_MAX 4

void pad_init(void);
uint8_t pad_read(void);
uint8_t buttons_read(void);

void led_init(void);
void board_init(void);
void led_on(void);
void led_off(void);
void led_toggle(void);

#endif
