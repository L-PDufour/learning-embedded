# STM32 Synth - multi-target build
#
# One compiler per target:
#   make       -> ARM firmware (arm-none-eabi-gcc)
#   make cli   -> desktop host binary (gcc)           [dev loop]
#   make wasm  -> browser build (emcc)                [not wired yet]
#
# The first target (all) is what plain `make` builds.

CFLAGS_COMMON = -std=gnu11 -Wall -Wextra -I Inc

# ---- ARM firmware ----
CC_ARM = arm-none-eabi-gcc
ARM_CFLAGS = -c -mcpu=cortex-m4 -mthumb -DSTM32F446xx -fno-builtin \
             -Ichip_headers/CMSIS/Device/ST/STM32F4xx/Include \
             -Ichip_headers/CMSIS/Include
ARM_LDFLAGS = -mcpu=cortex-m4 -mthumb -nostdlib \
              -T stm32_ls.ld \
              -Wl,-Map=synth.map
ARM_LIBS = -lc -lnosys -lgcc
ARM_SRCS = $(wildcard src/*.c) $(wildcard *.c)
ARM_TARGET = synth.elf

all: $(ARM_TARGET)

$(ARM_TARGET): $(ARM_SRCS:.c=.o)
	$(CC_ARM) $(ARM_LDFLAGS) $^ $(ARM_LIBS) -o $@

# Compile src/*.c files
src/%.o: src/%.c
	$(CC_ARM) $(CFLAGS_COMMON) $(ARM_CFLAGS) $< -o $@

# Compile root *.c files
%.o: %.c
	$(CC_ARM) $(CFLAGS_COMMON) $(ARM_CFLAGS) $< -o $@

# ---- Host (desktop) ----
CC_CLI = gcc
LDLIBS_CLI = -lm

cli:
	$(CC_CLI) $(CFLAGS_COMMON) main.c engine/engine.c src/platform_cli.c -o synth $(LDLIBS_CLI)

# ---- WASM (later) ----
CC_WASM = emcc

wasm:
	@echo "WASM target not wired up yet"

# ---- Flash & clean ----
flash:
	openocd -f board/st_nucleo_f4.cfg -c "init; reset halt; program $(ARM_TARGET) verify reset exit"

clean:
	rm -f *.o src/*.o *.elf *.map synth
