{ pkgs ? import <nixpkgs> {} }:
pkgs.stdenv.mkDerivation {
  pname = "chill";
  version = "1.0";

  src= ./.;

  nativeBuildInputs = with pkgs; [ clang ninja meson ];

  meta = with pkgs.lib; {
    description = "Just a chill shell";
    license = licenses.mit;
    maintainers = [ "FouFou" ];
    platforms = platforms.unix;
  };
}
