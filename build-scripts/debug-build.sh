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
