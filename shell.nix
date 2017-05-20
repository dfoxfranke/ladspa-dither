{ pkgs ? import <nixpkgs> {} }:
with pkgs;
stdenv.mkDerivation {
  name = "ladspa-dither";
  buildInputs = [ ladspa-sdk ];
}
