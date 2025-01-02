#!/bin/bash
# build.sh

# Kill any existing ran process
pkill ran

# Initial build
make clean && make all

# Store current directory
ROOT_DIR=$(pwd)

# Start ran in build directory in background
cd build/clay && ran -p 8000 &
RAN_PID=$!

# Go back to root for watching
cd "$ROOT_DIR"

# Trap to kill ran on script exit
trap 'pkill ran; exit 0' INT TERM EXIT

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
echo "Watching for changes in $ROOT_DIR/src"
while inotifywait -r -e modify "$ROOT_DIR/src"; do
    make clean && make all
done