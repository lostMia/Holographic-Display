{
  description = "PlatformIO environment using FHS user env for compatibility";

  # Inputs bring in the specific nixpkgs commit
  inputs = {
    piopatch.url = "https://github.com/NixOS/nixpkgs/archive/a1cd5e36101993f28efecc851a1152665420c8c6.tar.gz";
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";
  };

  outputs = { self, nixpkgs, piopatch }: {
    # Add architecture-specific outputs for x86_64-linux
    devShells.x86_64-linux.default = 
    let
      pkgs = import nixpkgs { system = "x86_64-linux"; };
      piopkg = import piopatch { system = "x86_64-linux"; };

      # Custom Python environment with PlatformIO
      mypython = pkgs.python3.withPackages (ps: with ps; [ piopkg.platformio ]);

    in pkgs.buildFHSUserEnv {
      name = "platformio-fhs";
      targetPkgs = pkgs: (with pkgs; [
        piopkg.platformio-core
        pkgs.fish 
        pkgs.neovide
        mypython
      ]);

      runScript = ''
        env LD_LIBRARY_PATH= fish -c "neovide; exec fish"
      '';
    };
  };
}
