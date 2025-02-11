#!/usr/bin/env bash

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

# First build rocks as a static library
echo "Building rocks library..."

# Compile rocks main source files
for file in ../rocks/src/*.c; do
  if [ -f "$file" ]; then
    echo "Compiling $file..."
    gcc -c $file -o build/clay/$(basename $file .c).o \
      $SDL_FLAGS \
      -I../rocks/include \
      -I../rocks/clay \
      -DROCKS_USE_SDL2 \
      -D_CRT_SECURE_NO_WARNINGS
  fi
done

# Compile rocks components
for file in ../rocks/src/components/*.c; do
  if [ -f "$file" ]; then
    echo "Compiling $file..."
    gcc -c $file -o build/clay/$(basename $file .c).o \
      $SDL_FLAGS \
      -I../rocks/include \
      -I../rocks/clay \
      -DROCKS_USE_SDL2 \
      -D_CRT_SECURE_NO_WARNINGS
  fi
done

# Compile SDL renderer files
for file in ../rocks/src/renderer/sdl2_*.c; do
  if [ -f "$file" ]; then
    echo "Compiling $file..."
    gcc -c $file -o build/clay/$(basename $file .c).o \
      $SDL_FLAGS \
      -I../rocks/include \
      -I../rocks/clay \
      -DROCKS_USE_SDL2 \
      -D_CRT_SECURE_NO_WARNINGS
  fi
done

# Create rocks static library
echo "Creating rocks static library..."
ar rcs build/clay/librocks.a build/clay/*.o

# Now build Quest source files
for file in src/*.c src/*/*.c src/*/*/*.c; do
  if [ -f "$file" ]; then
    echo "Compiling $file..."
    gcc -c $file -o build/clay/$(basename $file .c).o \
      $SDL_FLAGS \
      $INCLUDE_FLAGS \
      $COMMON_FLAGS
  fi
done

# Compile cJSON
if [ -f "vendor/cJSON/cJSON.c" ]; then
  echo "Compiling cJSON..."
  gcc -c vendor/cJSON/cJSON.c -o build/clay/cJSON.o \
    $INCLUDE_FLAGS \
    $COMMON_FLAGS
fi

# Link everything together
echo "Linking..."
gcc build/clay/*.o build/clay/librocks.a -o build/clay/quest \
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