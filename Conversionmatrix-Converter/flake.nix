{
  description = "Flake for building a conversion matrix for the holographic display.";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      pkgs = import nixpkgs { system = "x86_64-linux"; };
    in
    {
      packages.x86_64-linux.default = pkgs.stdenv.mkDerivation {
        pname = "conversion-matrix";
        version = "0.1.0";

        src = ./.;

        buildInputs = [ pkgs.gcc ];

        # Define build steps
        buildPhase = ''
          g++ -o main src/main.cpp
        '';

        # Define install steps
        installPhase = ''
          mkdir -p $out/bin
          cp main $out/bin/
        '';
      };
    };
}
