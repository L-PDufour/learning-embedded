{
  description = "STM32 synth: LLVM-only ARM Cortex-M + x86 host dev shell";

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
        llvm = pkgs.llvmPackages;

        # Unwrapped clang for bare-metal ARM: the Nix cc-wrapper injects
        # host glibc flags that break --target=arm-none-eabi. A symlink (not a
        # shell script) keeps bear from recording a duplicate nested exec.
        armClang = pkgs.linkFarm "arm-clang" [
          {
            name = "bin/arm-clang";
            path = "${llvm.clang-unwrapped}/bin/clang";
          }
        ];
      in
      {
        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            clang # host compiler (wrapped): `clang`
            armClang # ARM compiler (unwrapped): `arm-clang`
            llvm.lld # linker
            llvm.bintools # llvm-objdump, llvm-size, llvm-objcopy, llvm-nm
            llvm.lldb
            clang-tools # clangd, clang-format, clang-tidy
            bear
            cppcheck
            gnumake # still needed to run your Makefile
            openocd
            stlink
            picocom
          ];

          shellHook = ''
            export CLANGD_FLAGS="--background-index"
            echo "LLVM ARM + x86 dev shell (make / make cli / make flash)"
          '';
        };
      }
    );
}
