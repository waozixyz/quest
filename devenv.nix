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

  # Raylib build script
  scripts.build.exec = ''
    # Ensure build directory exists
    mkdir -p build/clay

    # Get Raylib flags
    RAYLIB_FLAGS=$(pkg-config --cflags raylib)
    RAYLIB_LIBS=$(pkg-config --libs raylib)

    echo "Building Quest with Raylib..."

    # Common include flags
    INCLUDE_FLAGS="-I./include \
      -I./include/pages \
      -I./include/components \
      -I./include/state \
      -I../rocks/include \
      -I../rocks/include/renderer \
      -I../rocks/include/components \
      -I../rocks/clay \
      -I./vendor/cJSON"

    # Common compiler flags
    COMMON_FLAGS="-Wall -Werror -O2 \
      -Wno-unused-variable \
      -Wno-missing-braces \
      -Wno-unused-but-set-variable \
      -DROCKS_USE_RAYLIB \
      -DCLAY_DESKTOP"

    # 1. src/**.c - all .c files in src and subdirectories
    for file in src/*.c src/*/*.c src/*/*/*.c; do
      if [ -f "$file" ]; then
        echo "Compiling $file..."
        ccache gcc -c $file -o build/$(basename $file .c).o \
          $RAYLIB_FLAGS \
          $INCLUDE_FLAGS \
          $COMMON_FLAGS
      fi
    done

    # 2. ../rocks/src/*.c and ../rocks/src/components/*.c
    for file in ../rocks/src/*.c ../rocks/src/components/*.c; do
      if [ -f "$file" ]; then
        echo "Compiling $file..."
        ccache gcc -c $file -o build/$(basename $file .c).o \
          $RAYLIB_FLAGS \
          $INCLUDE_FLAGS \
          $COMMON_FLAGS
      fi
    done

    # 3. Raylib renderer files
    for file in ../rocks/src/renderer/raylib_*.c; do
      if [ -f "$file" ]; then
        echo "Compiling $file..."
        ccache gcc -c $file -o build/$(basename $file .c).o \
          $RAYLIB_FLAGS \
          $INCLUDE_FLAGS \
          $COMMON_FLAGS
      fi
    done

    # 4. cJSON
    if [ -f "vendor/cJSON/cJSON.c" ]; then
      echo "Compiling cJSON..."
      ccache gcc -c vendor/cJSON/cJSON.c -o build/cJSON.o \
        $INCLUDE_FLAGS \
        $COMMON_FLAGS
    fi

    # Link everything together
    echo "Linking..."
    gcc build/*.o -o build/clay/quest \
      $RAYLIB_FLAGS $RAYLIB_LIBS \
      -lm -ldl -lpthread

    # Copy assets
    echo "Copying assets..."
    mkdir -p build/clay/fonts
    mkdir -p build/clay/images
    if [ -d "fonts" ]; then
      cp -r fonts/* build/clay/fonts/
    fi
    if [ -d "images" ]; then
      cp -r images/* build/clay/images/
    fi

    echo "Build completed!"
  '';

  # SDL2 build script
  scripts.build-sdl.exec = ''
    # Ensure build directory exists
    mkdir -p build/clay

    # Get SDL flags
    SDL_FLAGS=$(pkg-config --cflags sdl2 SDL2_ttf SDL2_image SDL2_gfx)
    SDL_LIBS=$(pkg-config --libs sdl2 SDL2_ttf SDL2_image SDL2_gfx)

    echo "Building Quest with SDL2..."

    # Common include flags

    INCLUDE_FLAGS="-I./include \
      -I./include/pages \
      -I./include/components \
      -I./include/state \
      -I../rocks/include \
      -I../rocks/include/renderer \
      -I../rocks/include/components \
      -I../rocks/clay \
      -I./vendor/cJSON"


    # Common compiler flags
    COMMON_FLAGS="-Wall -Werror -O2 \
      -Wno-unused-variable \
      -Wno-missing-braces \
      -Wno-unused-but-set-variable \
      -DROCKS_USE_SDL2 \
      -DCLAY_DESKTOP"

    # 1. src/**.c - all .c files in src and subdirectories
    for file in src/*.c src/*/*.c src/*/*/*.c; do
      if [ -f "$file" ]; then
        echo "Compiling $file..."
        ccache gcc -c $file -o build/$(basename $file .c).o \
          $SDL_FLAGS \
          $INCLUDE_FLAGS \
          $COMMON_FLAGS
      fi
    done

    # 2. ../rocks/src/*.c and ../rocks/src/components/*.c
    for file in ../rocks/src/*.c ../rocks/src/components/*.c; do
      if [ -f "$file" ]; then
        echo "Compiling $file..."
        ccache gcc -c $file -o build/$(basename $file .c).o \
          $SDL_FLAGS \
          $INCLUDE_FLAGS \
          $COMMON_FLAGS
      fi
    done

    # 3. SDL2 renderer files
    for file in ../rocks/src/renderer/sdl2_*.c; do
      if [ -f "$file" ]; then
        echo "Compiling $file..."
        ccache gcc -c $file -o build/$(basename $file .c).o \
          $SDL_FLAGS \
          $INCLUDE_FLAGS \
          $COMMON_FLAGS
      fi
    done

    # 4. cJSON
    if [ -f "vendor/cJSON/cJSON.c" ]; then
      echo "Compiling cJSON..."
      ccache gcc -c vendor/cJSON/cJSON.c -o build/cJSON.o \
        $INCLUDE_FLAGS \
        $COMMON_FLAGS
    fi

    # Link everything together
    echo "Linking..."
    gcc build/*.o -o build/clay/quest \
      $SDL_FLAGS $SDL_LIBS \
      -lm -ldl -lpthread

    # Copy assets
    echo "Copying assets..."
    mkdir -p build/clay/fonts
    mkdir -p build/clay/images
    if [ -d "fonts" ]; then
      cp -r fonts/* build/clay/fonts/
    fi
    if [ -d "images" ]; then
      cp -r images/* build/clay/images/
    fi

    echo "Build completed!"
  '';
  # Debug build with regular debug symbols
  scripts.debug-build.exec = ''
    # Ensure build directory exists
    mkdir -p build/clay

    # Get Raylib flags
    RAYLIB_FLAGS=$(pkg-config --cflags raylib)
    RAYLIB_LIBS=$(pkg-config --libs raylib)

    echo "Building Quest (Debug) with Raylib..."

    INCLUDE_FLAGS="-I./include \
      -I./include/pages \
      -I./include/components \
      -I./include/state \
      -I../rocks/include \
      -I../rocks/include/renderer \
      -I../rocks/include/components \
      -I../rocks/clay \
      -I./vendor/cJSON"


    # Debug compiler flags (without ASan)
    DEBUG_FLAGS="-Wall -Werror -g \
      -Wno-unused-variable \
      -Wno-missing-braces \
      -Wno-unused-but-set-variable \
      -DROCKS_USE_RAYLIB \
      -DCLAY_DESKTOP \
      -O1"

    # Compile everything with debug symbols
    # 1. src/**.c files
    for file in src/*.c src/*/*.c src/*/*/*.c; do
      if [ -f "$file" ]; then
        echo "Compiling $file..."
        ccache gcc -c $file -o build/$(basename $file .c).o \
          $RAYLIB_FLAGS \
          $INCLUDE_FLAGS \
          $DEBUG_FLAGS
      fi
    done

    # 2. ../rocks/src/*.c and components
    for file in ../rocks/src/*.c ../rocks/src/components/*.c; do
      if [ -f "$file" ]; then
        echo "Compiling $file..."
        ccache gcc -c $file -o build/$(basename $file .c).o \
          $RAYLIB_FLAGS \
          $INCLUDE_FLAGS \
          $DEBUG_FLAGS
      fi
    done

    # 3. Raylib renderer files
    for file in ../rocks/src/renderer/raylib_*.c; do
      if [ -f "$file" ]; then
        echo "Compiling $file..."
        ccache gcc -c $file -o build/$(basename $file .c).o \
          $RAYLIB_FLAGS \
          $INCLUDE_FLAGS \
          $DEBUG_FLAGS
      fi
    done

    # 4. cJSON
    if [ -f "vendor/cJSON/cJSON.c" ]; then
      echo "Compiling cJSON..."
      ccache gcc -c vendor/cJSON/cJSON.c -o build/cJSON.o \
        $INCLUDE_FLAGS \
        $DEBUG_FLAGS
    fi

    # Link everything together
    echo "Linking..."
    gcc build/*.o -o build/clay/quest \
      $RAYLIB_FLAGS $RAYLIB_LIBS \
      -lm -ldl -lpthread

    # Copy assets
    echo "Copying assets..."
    mkdir -p build/clay/fonts
    mkdir -p build/clay/images
    if [ -d "fonts" ]; then
      cp -r fonts/* build/clay/fonts/
    fi
    if [ -d "images" ]; then
      cp -r images/* build/clay/images/
    fi

    echo "Debug build completed!"
    echo "To debug memory issues run: valgrind --leak-check=full ./build/clay/quest"
    echo "To debug with GDB run: gdb ./build/clay/quest"
  '';

  scripts.clean.exec = ''
    rm -rf build
    echo "Clean completed successfully!"
  '';

  enterShell = ''
    echo "Quest development environment ready!"
    echo "Available commands:"
    echo "  build        - Build the project with Raylib (optimized)"
    echo "  build-sdl    - Build the project with SDL2 (optimized)"
    echo "  debug-build  - Build with Raylib and Address Sanitizer"
    echo "  clean        - Clean build artifacts"
  '';
}