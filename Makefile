# STM32 Synth - multi-target build
#
# One compiler per target:
#   make       -> ARM firmware (arm-none-eabi-gcc)
#   make cli   -> desktop host binary (gcc)           [dev loop]
#   make wasm  -> browser build (emcc)                [not wired yet]
#
# The first target (all) is what plain `make` builds.

CFLAGS_COMMON = -std=c99 -Wall -Wextra -Wpedantic -Werror -I Inc

# ---- ARM firmware ----
CC_ARM = arm-none-eabi-gcc
CPU_FLAGS = -mcpu=cortex-m4 -mthumb
FPU_FLAGS = -mfpu=fpv4-sp-d16 -mfloat-abi=hard
ARM_CFLAGS = -c -g $(CPU_FLAGS) $(FPU_FLAGS) -DSTM32F411xE -fno-builtin \
             -Ichip_headers/CMSIS/Device/ST/STM32F4xx/Include \
             -Ichip_headers/CMSIS/Include
ARM_LDFLAGS = $(CPU_FLAGS) $(FPU_FLAGS) -nostdlib \
              -T stm32_ls.ld \
              -Wl,-Map=synth.map
ARM_LIBS = -lm -lc -lnosys -lgcc
ARM_SRCS = stm32f411_startup.c main.c src/platform_stm.c src/systick.c \
	   src/gpio.c src/i2c.c src/codec.c src/i2s.c engine/engine.c src/i2s_dma.c
ARM_OBJS = $(ARM_SRCS:.c=.o)
ARM_TARGET = synth.elf

all: $(ARM_TARGET)

$(ARM_TARGET): $(ARM_OBJS)
	$(CC_ARM) $(ARM_LDFLAGS) $^ $(ARM_LIBS) -o $@

# Any .c anywhere (root, src/, engine/) compiles the same way.
%.o: %.c
	$(CC_ARM) $(CFLAGS_COMMON) $(ARM_CFLAGS) $< -o $@

# ---- Host (desktop) ----
CC_CLI = gcc
CLI_SRCS = main.c engine/engine.c src/platform_cli.c
LDLIBS_CLI = -lm

cli: synth

synth: $(CLI_SRCS)
	$(CC_CLI) $(CFLAGS_COMMON) $(CLI_SRCS) -o $@ $(LDLIBS_CLI)

# ---- WASM (later) ----
CC_WASM = emcc

wasm:
	@echo "WASM target not wired up yet"

# ---- Flash & clean ----
# F411E-DISCO uses the ST-Link interface + the generic F4 target script
# (there is no board/stm32f411discovery.cfg).
OPENOCD = openocd
OCD_IFACE = interface/stlink.cfg
OCD_TARGET = target/stm32f4x.cfg

flash: $(ARM_TARGET)
	$(OPENOCD) -f $(OCD_IFACE) -f $(OCD_TARGET) \
	  -c "init; reset halt; program $(ARM_TARGET) verify reset exit"

clean:
	rm -f $(ARM_OBJS) *.o src/*.o engine/*.o $(ARM_TARGET) *.map synth

.PHONY: all cli wasm flash clean
