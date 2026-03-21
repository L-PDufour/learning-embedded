#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>

#define SSD1306_ADDR 0x3C
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64

void ssd1306_init(void);
void ssd1306_update(void); // flush framebuffer to display
void ssd1306_clear(void);
void ssd1306_draw_pixel(int x, int y, uint8_t on);
void ssd1306_draw_char(int x, int page, char c);
void ssd1306_draw_string(int x, int page, const char *str);
void ssd1306_fill(uint8_t val);
#endif
