#!/bin/bash
# build.sh

# Kill any existing ran process
pkill serve

# Initial build
xmake f -p wasm
xmake

# Store current directory
ROOT_DIR=$(pwd)

# Start ran in build directory in background
cd build/clay && serve &
RAN_PID=$!

# Go back to root for watching
cd "$ROOT_DIR"

# Trap to kill ran on script exit
trap 'pkill serve; exit 0' INT TERM EXIT

# Check if inotifywait exists
if ! command -v inotifywait &> /dev/null; then
    echo "inotifywait not found. Please install inotify-tools"
    echo "Ubuntu/Debian: sudo apt-get install inotify-tools"
    echo "macOS: brew install fswatch"
    # Just keep running without watch
    while true; do
        sleep 1
    done
fi

# Watch for changes and rebuild

echo "Watching for changes in $ROOT_DIR/src and index.html"
while inotifywait -r -e modify "$ROOT_DIR/src" "$ROOT_DIR/index.html" "$ROOT_DIR/scripts"; do
   xmake f -p wasm && xmake
done
