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
              mold      # A much faster linker than the rest

              # Some optional python scripts require python with some packages
              (python3.withPackages (p: with p; [
                pip
                virtualenv
              ]))
          ];
          shellHook = ''
             # Configure env vars
             export CC="${pkgs.gcc14}/bin/gcc"
             export CXX="${pkgs.gcc14}/bin/g++"

             # Set up mold as the linker
             export LD="${pkgs.mold}/bin/mold"

             # Create a directory for symlinks
             mkdir -p $HOME/.local/bin

             # Create symlinks to mold
             ln -sf ${pkgs.mold}/bin/mold $HOME/.local/bin/ld
             ln -sf ${pkgs.mold}/bin/mold $HOME/.local/bin/ld.lld

             # Modify PATH to prioritize our symlinks, gcc14, and mold
             export PATH="$HOME/.local/bin:${pkgs.mold}/bin:${pkgs.gcc14}/bin:$PATH"

             # Configure GCC to use mold
             export LDFLAGS="-fuse-ld=mold"
             export NIX_CFLAGS_LINK="-B${pkgs.mold}/lib"
             export NIX_LDFLAGS="$NIX_LDFLAGS -fuse-ld=mold"

             echo 'Welcome to the equilibrium dev shell'
             echo 'Mold is set as the linker'
           '';
        };
      });
}
