#!/bin/bash
# run_linux.sh

# Get current architecture
ARCH=$(uname -m)
case "$ARCH" in
    "x86_64") XMAKE_ARCH="x86_64" ;;
    "aarch64") XMAKE_ARCH="aarch64" ;;
    "armv7l") XMAKE_ARCH="armv7" ;;
    *) echo "Unsupported architecture: $ARCH" && exit 1 ;;
esac

# Build SDL dependencies if they don't exist
if [ "$XMAKE_ARCH" != "x86_64" ] && [ ! -d "vendor/linux-$XMAKE_ARCH" ]; then
    echo "Building SDL dependencies for $XMAKE_ARCH..."
    xmake build_sdl_linux --arch=$XMAKE_ARCH
fi

# Clean previous build
xmake clean

# Configure and build with new packages
xmake f -p linux -a $XMAKE_ARCH
xmake

# Run the executable
if [ "$XMAKE_ARCH" = "x86_64" ]; then
    ./build/clay/main
else
    ./build/clay/$XMAKE_ARCH/main
fi