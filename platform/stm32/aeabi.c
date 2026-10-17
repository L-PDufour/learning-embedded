#include <stddef.h>

/*
 * Clang emits AEABI runtime helpers for struct copies on ARM. The bare-metal
 * toolchain has no ARM compiler-rt archive, so provide the one this firmware
 * needs (currently only the Engine return from engine_init).
 */
void *__aeabi_memcpy4(void *dest, const void *src, size_t n) {
  unsigned char *d = (unsigned char *)dest;
  const unsigned char *s = (const unsigned char *)src;
  size_t i;

  for (i = 0; i < n; i++) {
    d[i] = s[i];
  }
  return dest;
}
