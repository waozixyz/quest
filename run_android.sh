#!/bin/sh
set -e

echo "Building Android app..."
make build-android

echo "Installing APKs..."
cd android/app/build/outputs/apk/debug

# Get device ABI
DEVICE_ABI=$(adb shell getprop ro.product.cpu.abi)
echo "Device ABI: $DEVICE_ABI"

# Choose appropriate APK based on device ABI
case $DEVICE_ABI in
    "arm64-v8a")
        APK="app-arm64-v8a-debug.apk"
        ;;
    "armeabi-v7a")
        APK="app-armeabi-v7a-debug.apk"
        ;;
    "x86_64")
        APK="app-x86_64-debug.apk"
        ;;
    "x86")
        APK="app-x86-debug.apk"
        ;;
    *)
        echo "Unknown ABI, using universal APK"
        APK="app-universal-debug.apk"
        ;;
esac

echo "Installing $APK..."
adb install -r $APK
cd ../../../../../

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
    