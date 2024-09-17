{
  description = "Equilibrium building environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";

    # For Windows builds, we need to use a MSVC wrapper
    # that ships with a copy of the MSVC libraries.
    msvc-wine.url = "github:roblabla/msvc-llvm-nix";
  };

  outputs = { self, nixpkgs, flake-utils, msvc-wine }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        overlays = [
          msvc-wine.overlay
        ];
        pkgs = import nixpkgs {
          inherit system overlays;
          config.allowUnfree = true;
        };

        # Common packages for both Linux and Windows
        commonPackages = with pkgs; [
          gnumake
          (python3.withPackages (p: with p; [
            pip
            virtualenv
          ]))
        ];

        # Linux-specific packages and configuration
        linuxPackages = with pkgs; [
          gcc14
          libclang
          mold
          lldb_18
        ];

        linuxShellHook = ''
          export CC="${pkgs.gcc14}/bin/gcc"
          export CXX="${pkgs.gcc14}/bin/g++"
          export LD="${pkgs.mold}/bin/mold"
          mkdir -p $HOME/.local/bin
          ln -sf ${pkgs.mold}/bin/mold $HOME/.local/bin/ld
          ln -sf ${pkgs.mold}/bin/mold $HOME/.local/bin/ld.lld
          export PATH="$HOME/.local/bin:${pkgs.mold}/bin:${pkgs.gcc14}/bin:$PATH"
          export LDFLAGS="-fuse-ld=mold"
          export NIX_CFLAGS_LINK="-B${pkgs.mold}/lib"
          export NIX_LDFLAGS="$NIX_LDFLAGS -fuse-ld=mold"
          export EQUILIBRIUM_NIX=1
          echo 'Linux build environment loaded'
          echo 'Mold is set as the linker'
        '';

        # Windows-specific packages and configuration
        windowsPackages = with pkgs; [
          msvc-toolchain
          cmake
          msbuild
        ];

        windowsShellHook = ''
          export PATH=${pkgs.msvc-toolchain}/bin/x64:$PATH
          export BIN=${pkgs.msvc-toolchain}/bin/x64
          . $BIN/msvcenv.sh
          export CC=clang-cl
          export CXX=clang-cl
          export LD=lld-link
          echo 'Windows build environment loaded with MSVC toolchain'
        '';

      in
      {
        devShells = {
          linux = pkgs.mkShell {
            name = "equilibrium linux dev shell";
            LD_LIBRARY_PATH = "${pkgs.stdenv.cc.cc.lib}/lib";
            buildInputs = commonPackages ++ linuxPackages;
            shellHook = linuxShellHook;
          };

          wincross = pkgs.mkShell {
            name = "equilibrium windows dev shell";
            buildInputs = commonPackages ++ windowsPackages;
            shellHook = windowsShellHook;
          };

          default = pkgs.mkShell {
            name = "equilibrium dev shell";
            LD_LIBRARY_PATH = "${pkgs.stdenv.cc.cc.lib}/lib";
            buildInputs = commonPackages ++ linuxPackages ++ windowsPackages;
            shellHook = ''
              echo 'Welcome to the equilibrium dev shell'
              echo 'Use "nix develop .#linux" for Linux build env'
              echo 'Use "nix develop .#wincross" for Linux for Windows crosscompile build env'
            '';
          };
        };
      });
}
