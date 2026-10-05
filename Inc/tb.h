#ifndef TB_H
#define TB_H

#include <stdint.h>

/*
 * TigerStyle assertions for a bare-metal target.
 *
 * Assertions flag programmer errors (invariants, pre/postconditions) and must
 * crash. Operating errors (I2C NACK/timeout) are expected and are returned as
 * status values instead, never asserted.
 */

/* Compile-time assertion. C89 has no _Static_assert, so use the typedef trick.
 */
#define TB_STATIC_ASSERT(condition, name)                                      \
  typedef char tb_static_assert_##name[(condition) ? 1 : -1]

#define TB_ASSERT(condition)                                                   \
  do {                                                                         \
    if (!(condition))                                                          \
      tb_panic(__FILE__, __LINE__);                                            \
  } while (0)

#define TB_UNREACHABLE() tb_panic(__FILE__, __LINE__)

/* Never returns. Latches the source location for the debugger, then halts. */
__attribute__((noreturn)) void tb_panic(const char *file, uint32_t line);

#endif
