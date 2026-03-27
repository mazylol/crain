{
  description = "crain - rain in the terminal using ncurses";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        packages = {
          crain = pkgs.stdenv.mkDerivation {
            pname = "crain";
            version = "0.1.0";

            src = ./.;

            nativeBuildInputs = [ pkgs.clang ];
            buildInputs = [ pkgs.ncurses ];

            buildPhase = ''
              # Bootstrap nob build system
              clang nob.c -o nob
              ./nob
            '';

            installPhase = ''
              mkdir -p $out/bin
              cp build/crain $out/bin/
            '';

            meta = with pkgs.lib; {
              description = "Rain in the terminal using ncurses";
              homepage = "https://github.com/mazylol/crain";
              license = licenses.gpl3Plus;
              maintainers = [ ];
              platforms = platforms.unix;
            };
          };

          default = self.packages.${system}.crain;
        };

        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            clang
            ncurses
          ];
        };
      }
    );
}
