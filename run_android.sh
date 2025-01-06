#!/bin/sh
set -e

echo "Building Android app..."
make build-android

echo "Installing APK..."
adb install -r android/app/build/outputs/apk/debug/app-debug.apk

echo "Starting app..."
adb shell am start -n xyz.waozi.myquest/.MainActivity

echo "Showing logs..."
adb logcat -c
adb logcat \
    SDL:V \
    MainActivity:V \
    SDL_APP:V \
    Clay:V \
    AndroidRuntime:E \
    libc:V \
    DEBUG:V \
    native:V \
    stderr:V \
    *:E \
    *:S \
    *:I \
    | grep -v -E "SignalStrength|NetworkController|DisplayPowerController|MotionDetector|WindowManager|InputReader|InputDispatcher|Counters|VerifyUtils|Finsky|HWComposer"