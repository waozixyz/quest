#!/bin/sh
set -e

# Set NDK path
export NDK_PATH="/opt/android-ndk"

# Get device ABI
DEVICE_ABI=$(adb shell getprop ro.product.cpu.abi)
echo "Device ABI: $DEVICE_ABI"

# Validate ABI
case $DEVICE_ABI in
    "arm64-v8a"|"armeabi-v7a"|"x86_64"|"x86")
        echo "Building Android app for $DEVICE_ABI..."
        # Build SDL dependencies for specific ABI
        xmake build_sdl_android --ARCH=$DEVICE_ABI
        # Configure xmake for Android with specific ABI
        xmake f -p android --arch=$DEVICE_ABI --ndk=$NDK_PATH
        # Build the project
        xmake build
        # Build the APK using Gradle
        cd android && ./gradlew assembleDebug
        ;;
    *)
        echo "Unsupported ABI: $DEVICE_ABI"
        exit 1
        ;;
esac
echo "Installing APKs..."
cd app/build/outputs/apk/debug

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
esac

echo "Installing $APK..."
adb install -r "$APK"

cd ../../../../

echo "Starting app..."
adb shell am start -n io.naox.quest/.MainActivity

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