{
  description = "ARM Cortex-M embedded C development environment";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            # Cross-compiler toolchain (ARM bare-metal)
            gcc
            # Flashing & on-chip debugging
            openocd
            stlink # st-flash, st-info

            # Build
            gnumake

            # Language server (needs compile_commands.json — use bear to generate)
            clang-tools # clangd, clang-format, clang-tidy
            bear # bear -- make   →  compile_commands.json

            # Static analysis
            cppcheck

            # Serial monitor
            picocom
            alsa-utils

            # Binary inspection
            binutils-unwrapped
          ];

          shellHook = ''
            echo "⚙️  ARM Cortex-M development environment"
            echo "Toolchain: $(arm-none-eabi-gcc --version | head -n 1)"
            echo "OpenOCD:   $(openocd --version 2>&1 | head -n 1)"
            echo ""
            echo "Build:"
            echo "  make"
            echo "  bear -- make          # also generates compile_commands.json"
            echo ""
            echo "Flash (ST-Link):"
            echo "  openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \\"
            echo "          -c 'program build/firmware.elf verify reset exit'"
            echo ""
            echo "Serial monitor:"
            echo "  picocom /dev/ttyUSB0 -b 115200   # exit: C-a C-x"
            echo ""
            echo "Inspect binary:"
            echo "  arm-none-eabi-size build/firmware.elf"
            echo "  arm-none-eabi-objdump -d build/firmware.elf | less"
          '';

        };
      }
    );
}
