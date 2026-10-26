# STM32 Synth - multi-target build
#
# One compiler per target:
#   make       -> ARM firmware (arm-clang)
#   make cli   -> desktop host binary (clang)         [dev loop]
#   make wasm  -> browser build (emcc)                [not wired yet]
#
# The first target (all) is what plain `make` builds.


CC_CLI = clang

CFLAGS_COMMON = -std=c99 -Wall -Wextra -Wpedantic  -I Inc
CC_ARM = arm-clang --target=thumbv7em-none-eabihf
# ---- ARM firmware ----
CPU_FLAGS = -mcpu=cortex-m4 -mthumb
FPU_FLAGS = -mfpu=fpv4-sp-d16 -mfloat-abi=hard
ARM_LDFLAGS = $(CPU_FLAGS) $(FPU_FLAGS) -nostdlib -fuse-ld=lld \
              -T stm32_ls.ld -Wl,--gc-sections -Wl,-Map=synth.map
ARM_CFLAGS = -c -g -O2 $(CPU_FLAGS) $(FPU_FLAGS) -DSTM32F411xE -fno-builtin \
             -ffunction-sections -fdata-sections \
             -Ichip_headers/CMSIS/Device/ST/STM32F4xx/Include \
             -Ichip_headers/CMSIS/Include


ARM_SRCS = platform/stm32/stm32f411_startup.c main.c \
	   platform/stm32/platform_stm.c platform/stm32/systick.c engine/tb.c \
	   platform/stm32/gpio.c platform/stm32/i2c.c platform/stm32/codec.c \
	   platform/stm32/i2s.c engine/engine.c engine/filter.c \
	   platform/stm32/i2s_dma.c \
	   platform/stm32/ssd1306.c platform/stm32/adc.c engine/ui.c \
	   platform/stm32/aeabi.c

ARM_OBJS = $(ARM_SRCS:.c=.o)
ARM_TARGET = synth.elf

all: $(ARM_TARGET)

$(ARM_TARGET): $(ARM_OBJS)
	$(CC_ARM) $(ARM_LDFLAGS) $^ -o $@

# Any .c anywhere (root, engine/, platform/*/) compiles the same way.
%.o: %.c
	$(CC_ARM) $(CFLAGS_COMMON) $(ARM_CFLAGS) $< -o $@

# ---- Host (desktop) ----
CLI_SRCS = main.c engine/engine.c engine/filter.c platform/desktop/platform_cli.c \
	   engine/tb.c
LDLIBS_CLI = -lm

cli: synth

synth: $(CLI_SRCS)
	$(CC_CLI) $(CFLAGS_COMMON) $(CLI_SRCS) -o $@ $(LDLIBS_CLI)

# ---- Debug firmware ----
# Same sources, -O0, separate objects and ELF so the normal build and flash are
# never disturbed. The audio ISR is far too slow at -O0 to stay glitch-free:
# use this build to inspect code and step lines, not to listen.
ARM_DBG_OBJS = $(ARM_SRCS:.c=.dbo)

%.dbo: %.c
	$(CC_ARM) $(CFLAGS_COMMON) $(ARM_CFLAGS) -O0 $< -o $@

synth-debug.elf: $(ARM_DBG_OBJS)
	$(CC_ARM) $(ARM_LDFLAGS) $^ -o $@

debug: synth-debug.elf

debug-flash: synth-debug.elf
	$(OPENOCD) -f $(OCD_IFACE) -f $(OCD_TARGET) \
	  -c "init; reset halt; program synth-debug.elf verify reset exit"

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
	rm -f $(ARM_OBJS) $(ARM_DBG_OBJS) *.o engine/*.o platform/*/*.o \
	  $(ARM_TARGET) synth-debug.elf *.map synth

.PHONY: all cli debug debug-flash wasm flash clean
