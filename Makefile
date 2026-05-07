CC = arm-none-eabi-gcc
CFLAGS = -c -mcpu=cortex-m4 -mthumb -std=gnu11 -DSTM32F446xx \
         -fno-builtin \
         -IInc \
         -Ichip_headers/CMSIS/Device/ST/STM32F4xx/Include \
         -Ichip_headers/CMSIS/Include
LDFLAGS = -mcpu=cortex-m4 -mthumb -nostdlib \
          -T stm32_ls.ld \
          -Wl,-Map=$(TARGET).map

LIBS = -lc -lnosys -lgcc

TARGET = 5_makefile_project_v2

# Automatically find all .c files
SRCS = $(wildcard src/*.c) $(wildcard *.c)

# Automatically generate .o names from .c names
OBJS = $(patsubst %.c, %.o, $(SRCS))

# Default target
all: $(TARGET).elf

# Compile src/*.c files
src/%.o: src/%.c
	$(CC) $(CFLAGS) $< -o $@

# Compile root *.c files
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

synth: main.c engine/engine.c src/platform_cli.c
	gcc main.c engine/engine.c src/platform_cli.c -o synth -I Inc -lm

cli: main.c engine/engine.c src/platform_cli.c
	gcc main.c engine/engine.c src/platform_cli.c -o synth -I Inc -lm

$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

flash:
	openocd -f board/st_nucleo_f4.cfg -c "init; reset halt; program 5_makefile_project_v2.elf verify reset exit"

clean:
	rm -f *.o src/*.o *.elf *.map synth cli
