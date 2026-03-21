#include <stdint.h>
#include <sys/stat.h>

// these are the minimum stubs needed for printf to work
// without a full OS

int _write(int file, char *ptr, int len) {
  // this is called by printf internally
  // __io_putchar is already defined in uart.c
  extern int __io_putchar(int ch);
  for (int i = 0; i < len; i++) {
    __io_putchar(ptr[i]);
  }
  return len;
}

int _read(int file, char *ptr, int len) { return 0; }
int _close(int file) { return -1; }
int _fstat(int file, struct stat *st) { return 0; }
int _isatty(int file) { return 1; }
int _lseek(int file, int ptr, int dir) { return 0; }
void _exit(int status) {
  while (1) {
  }
}
int _kill(int pid, int sig) { return -1; }
int _getpid(void) { return 1; }
// minimal heap for printf's internal buffer
static uint8_t heap[512];
static uint8_t *heap_ptr = heap;
void *_sbrk(int incr) {
  uint8_t *prev = heap_ptr;
  heap_ptr += incr;
  return (void *)prev;
}
