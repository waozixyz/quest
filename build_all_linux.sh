#!/bin/bash
# build_all.sh

# Array of target triples
TARGETS=(
    "x86_64-linux-gnu"
    "aarch64-linux-gnu"
    "arm-linux-gnueabihf"
    "riscv64-linux-gnu"
)

# Clean previous builds
xmake clean

for target in "${TARGETS[@]}"; do
    echo "Building for $target..."
    
    case $target in
        "x86_64-linux-gnu")
            xmake f -p linux -a x86_64
            ;;
        "aarch64-linux-gnu")
            xmake f -p linux -a arm64 --sdk=/usr/aarch64-linux-gnu
            ;;
        "arm-linux-gnueabihf")
            xmake f -p linux -a armv7 --sdk=/usr/arm-linux-gnueabihf
            ;;
        "riscv64-linux-gnu")
            xmake f -p linux -a riscv64 --sdk=/usr/riscv64-linux-gnu
            ;;
    esac
    
    CFLAGS="--target=$target -D__ARM_FEATURE_SVE=0 -D__SSE__=0 -D__SSE2__=0 -D__AVX__=0" \
    LDFLAGS="--target=$target" \
    xmake
    
    # Create architecture-specific output directory
    arch=$(echo $target | cut -d'-' -f1)
    mkdir -p "build/clay/$arch"
    mv build/clay/main "build/clay/$arch/main" || true
done

echo "All builds completed!"