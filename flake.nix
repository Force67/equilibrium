{
  description = "Equilibrium building environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils}:
    flake-utils.lib.eachDefaultSystem (system:
      let
        overlays = [

        ];
        pkgs = import nixpkgs {
          inherit system overlays;
          config.allowUnfree = true;
        };
      in
      {
        devShells.default = pkgs.mkShell {
          name = "equilibrium dev shell";
          buildInputs = with pkgs; [
              gnumake   # to build premake outputs
              gcc14     # gcc compiler
              libclang  # clang compiler

              # Some optional python scripts require python with some packages
              (python3.withPackages (p: with p; [
                pip
                virtualenv
              ]))
          ];
          shellHook = ''
            # Configure env vars
            export CC=${pkgs.gcc14}/bin/gcc
            export CXX=${pkgs.gcc14}/bin/g++
            export LD=${pkgs.gcc14}/bin/gcc

            # Use gcc for linking
            export LDFLAGS="-fuse-ld=gcc"

            # Modify PATH to prioritize gcc14
            export PATH="${pkgs.gcc14}/bin:$PATH"

            # Create aliases for manual testing
            alias gcc='${pkgs.gcc14}/bin/gcc'
            alias g++='${pkgs.gcc14}/bin/g++'
            alias ld='${pkgs.gcc14}/bin/gcc'

            echo 'Welcome to the equilibrium dev shell'
          '';
        };
      });
}
