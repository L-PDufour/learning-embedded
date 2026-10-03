#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define SINE_TABLE_BITS 8
#define SINE_TABLE_SIZE (1 << SINE_TABLE_BITS)
#define MY_PI 3.14159265358979323846
#define SAMPLE_MAX 32767
#define AMPLITUDE (SAMPLE_MAX / 2)

int main(void) {
  int i = 0;
  double angle = 0;
  int v = 0;
  printf("#ifndef SINE_TABLE_H\n");
  printf("#define SINE_TABLE_H\n");
  printf("/*generated, do not edit */\n");
  printf("#include <stdint.h>\n");
  printf("#define SINE_TABLE_BITS 8\n");
  printf("#define SINE_TABLE_SIZE 256\n");
  printf("static const int16_t sine_table[SINE_TABLE_SIZE] = {\n");
  for (i = 0; i < SINE_TABLE_SIZE; i++) {
    angle = 2 * MY_PI * i / SINE_TABLE_SIZE;
    v = (int)lround(sin(angle) * AMPLITUDE);
    printf("%d,", v);
    if (i % 8 == 7)
      printf("\n");
  }
  printf("};\n");
  printf("#endif\n");
}
