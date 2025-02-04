#!/bin/bash
# run_linux.sh

# Function to display error messages and exit
error_exit() {
    echo "Error: $1"
    exit 1
}

# Get current architecture
ARCH=$(uname -m)
case "$ARCH" in
    "x86_64") XMAKE_ARCH="x86_64" ;;
    "aarch64") XMAKE_ARCH="aarch64" ;;
    "armv7l") XMAKE_ARCH="armv7" ;;
    *) error_exit "Unsupported architecture: $ARCH" ;;
esac

# Prompt user for renderer choice
echo "Choose a renderer backend:"
echo "1) SDL2"
echo "2) Raylib"
read -p "Enter your choice (1 or 2): " RENDERER_CHOICE

# Validate user input
case "$RENDERER_CHOICE" in
    1) RENDERER="sdl2" ;;
    2) RENDERER="raylib" ;;
    *) error_exit "Invalid choice. Please select 1 for SDL2 or 2 for Raylib." ;;
esac

# Build SDL dependencies if they don't exist (only for non-x86_64 architectures)
if [ "$XMAKE_ARCH" != "x86_64" ] && [ ! -d "vendor/linux-$XMAKE_ARCH" ]; then
    echo "Building SDL dependencies for $XMAKE_ARCH..."
    xmake build_sdl_linux --arch=$XMAKE_ARCH || error_exit "Failed to build SDL dependencies."
fi

# Clean previous build
echo "Cleaning previous build..."
xmake clean || error_exit "Failed to clean previous build."

# Configure and build with the selected renderer
echo "Configuring and building with $RENDERER renderer..."
xmake f -p linux -a $XMAKE_ARCH --renderer=$RENDERER || error_exit "Failed to configure project."
xmake || error_exit "Failed to build project."

# Run the executable
echo "Running the application..."
if [ "$XMAKE_ARCH" = "x86_64" ]; then
    ./build/clay/main || error_exit "Failed to run the application."
else
    ./build/clay/$XMAKE_ARCH/main || error_exit "Failed to run the application."
fi