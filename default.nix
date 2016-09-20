{pkgs ? ((import <nixpkgs> {}).pkgs)}:

let
  stdenv = pkgs.stdenv;
in stdenv.mkDerivation rec {
 name = "cds-env";
 version = "0.0.0.1";
 buildInputs = [
   pkgs.cmake
 ];
}
