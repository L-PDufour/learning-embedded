#include "tb.h"

const char *volatile tb_panic_file = 0;
volatile uint32_t tb_panic_line = 0;

__attribute__((noreturn)) void tb_panic(const char *file, uint32_t line) {
  tb_panic_file = file;
  tb_panic_line = line;
  for (;;) {
  }
}
