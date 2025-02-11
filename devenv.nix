{ pkgs, ... }:
{
  packages = with pkgs; [
    pkg-config
    SDL2
    SDL2_ttf
    SDL2_image
    SDL2_gfx
    raylib
    gcc
    gdb
    ccache
    valgrind
  ];

  languages.c.enable = true;

  # Reference scripts from build-scripts directory
  scripts.build.exec = (builtins.readFile ./build-scripts/build.sh);
  scripts.build-sdl.exec = (builtins.readFile ./build-scripts/build-sdl.sh);
  scripts.debug-build.exec = (builtins.readFile ./build-scripts/debug-build.sh);
  scripts.clean.exec = (builtins.readFile ./build-scripts/clean.sh);

  enterShell = ''
    echo "Quest development environment ready!"
    echo "Available commands:"
    echo "  build        - Build the project with Raylib (optimized)"
    echo "  build-sdl    - Build the project with SDL2 (optimized)"
    echo "  debug-build  - Build with Raylib and Address Sanitizer"
    echo "  clean        - Clean build artifacts"
  '';
}