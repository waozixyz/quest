#!/bin/bash
# run_linux.sh

# Clean previous build
xmake clean

# Configure and build with new packages
xmake f -p linux
xmake

# Run the executable
./build/clay/main