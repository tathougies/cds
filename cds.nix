{pkgs ? ((import <nixpkgs> {}).pkgs)}:

let stdenv = pkgs.stdenv;
in stdenv.mkDerivation rec {
  name = "cds";
  version = "0.0.0.1";
  src = builtins.filterSource (path: type: path != "default.nix" && path != "cds.nix") ./.;
  buildInputs = [ pkgs.cmake ];
}
