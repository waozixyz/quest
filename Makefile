# Build configuration
INCLUDE_DIR = include
INCLUDE_FLAGS = -I$(INCLUDE_DIR)
BUILD_DIR = build/clay
SRC_DIR = src
NDK_PATH ?= /opt/android-ndk
SDK_PATH ?= /opt/android-sdk
ANDROID_ABIS ?= armeabi-v7a arm64-v8a x86 x86_64

ifeq ($(BUILD_TYPE),web)
	CC = emcc
	CFLAGS = -Wall -Werror -Os -DCLAY_WASM -mbulk-memory --target=wasm32 
	LINKER_FLAGS = \
		-Wl,--strip-all \
		-Wl,--export-dynamic \
		-Wl,--export=__heap_base \
		-Wl,--export=ACTIVE_RENDERER_INDEX \
		-s WASM=1 \
		-s USE_PTHREADS=0 \
		-s ASSERTIONS=1 \
		-s ALLOW_MEMORY_GROWTH=1 \
		-s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
		-s EXPORTED_FUNCTIONS='["_main", "_printf"]'
	TARGET = $(BUILD_DIR)/index.wasm
	SRCS = $(shell find $(SRC_DIR) -name "*.c" ! -path "$(SRC_DIR)/renderers/*")
else ifeq ($(BUILD_TYPE),android)
    CFLAGS = -Wall -Werror -O2 -DCLAY_MOBILE -fPIC
    INCLUDE_FLAGS += \
        -I$(NDK_PATH)/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include \
        -Ivendor/cJSON \
        -Ivendor/SDL/include \
        -Ivendor/SDL_image/include \
        -Ivendor/SDL_ttf

    LINKER_FLAGS = -shared -landroid -llog -lm \
        -L$(dir $(TARGET)) \
        -lSDL2 -lSDL2_image -lSDL2_ttf

    SRCS = $(shell find $(SRC_DIR) -name "*.c")
    SRCS += vendor/cJSON/cJSON.c
else ifeq ($(BUILD_TYPE),desktop)
	CC = clang
	CFLAGS = -Wall -Werror -O2 -DCLAY_DESKTOP
	LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lm
	INCLUDE_FLAGS += -I/usr/include/SDL2 -Ivendor/cJSON
	TARGET = $(BUILD_DIR)/game
	SRCS = $(shell find $(SRC_DIR) -name "*.c")
	SRCS += vendor/cJSON/cJSON.c
endif

OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)

all: $(TARGET) copy_assets

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LINKER_FLAGS) $(INCLUDE_FLAGS) -o $@ $(OBJS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

copy_assets:
ifeq ($(BUILD_TYPE),web)
	cp index.html $(BUILD_DIR)/index.html
	cp manifest.json $(BUILD_DIR)/manifest.json
	cp -r fonts/ $(BUILD_DIR)/fonts
	cp -r images/ $(BUILD_DIR)/images
else ifeq ($(BUILD_TYPE),android)
	mkdir -p android/app/src/main/assets/fonts
	mkdir -p android/app/src/main/assets/images
	cp -r fonts/* android/app/src/main/assets/fonts/
	cp -r images/* android/app/src/main/assets/images/
else
	cp -r fonts/ $(BUILD_DIR)/fonts
	cp -r images/ $(BUILD_DIR)/images
endif

clean:
	rm -rf $(BUILD_DIR)

build-sdl-android:
	@for abi in $(ANDROID_ABIS) ; do \
		echo "Building SDL for $$abi" && \
		mkdir -p vendor/SDL/build-android-$$abi && \
		(cd vendor/SDL/build-android-$$abi && \
		cmake .. \
			-DCMAKE_TOOLCHAIN_FILE=$(NDK_PATH)/build/cmake/android.toolchain.cmake \
			-DANDROID_ABI=$$abi \
			-DANDROID_PLATFORM=android-21 \
			-DBUILD_SHARED_LIBS=ON && \
		make) && \
		mkdir -p android/app/src/main/jniLibs/$$abi/ && \
		cp vendor/SDL/build-android-$$abi/libSDL2.so android/app/src/main/jniLibs/$$abi/ ; \
	done

build-sdl-image-android: build-sdl-android
	@for abi in $(ANDROID_ABIS) ; do \
		echo "Building SDL_image for $$abi" && \
		mkdir -p vendor/SDL_image/build-android-$$abi && \
		(cd vendor/SDL_image/build-android-$$abi && \
		cmake .. \
			-DCMAKE_TOOLCHAIN_FILE=$(NDK_PATH)/build/cmake/android.toolchain.cmake \
			-DANDROID_ABI=$$abi \
			-DANDROID_PLATFORM=android-21 \
			-DSDL2_DIR=../../SDL/build-android-$$abi \
			-DSDL2_INCLUDE_DIR=../../SDL/include \
			-DSDL2_LIBRARY=../../SDL/build-android-$$abi/libSDL2.so \
			-DSDL2IMAGE_AVIF=OFF \
			-DSDL2IMAGE_WEBP=OFF \
			-DSDL2IMAGE_TIFF=OFF && \
		make) && \
		cp vendor/SDL_image/build-android-$$abi/libSDL2_image.so android/app/src/main/jniLibs/$$abi/ ; \
	done

build-sdl-ttf-android: build-sdl-image-android
	@for abi in $(ANDROID_ABIS) ; do \
		echo "Building SDL_ttf for $$abi" && \
		mkdir -p vendor/SDL_ttf/build-android-$$abi && \
		(cd vendor/SDL_ttf/build-android-$$abi && \
		cmake .. \
			-DCMAKE_TOOLCHAIN_FILE=$(NDK_PATH)/build/cmake/android.toolchain.cmake \
			-DANDROID_ABI=$$abi \
			-DANDROID_PLATFORM=android-21 \
			-DSDL2_DIR=../../SDL/build-android-$$abi \
			-DSDL2_INCLUDE_DIR=../../SDL/include \
			-DSDL2_LIBRARY=../../SDL/build-android-$$abi/libSDL2.so \
			-DBUILD_SHARED_LIBS=ON \
			-DSDL2TTF_SAMPLES=OFF && \
		make) && \
		cp vendor/SDL_ttf/build-android-$$abi/libSDL2_ttf.so android/app/src/main/jniLibs/$$abi/ ; \
	done

build-android-abi:
	rm -rf $(BUILD_DIR)
	$(eval CC = $(NDK_PATH)/toolchains/llvm/prebuilt/linux-x86_64/bin/$(TARGET_TRIPLE)-clang)
	@echo "Building for ABI: $(TARGET_ABI) using compiler: $(CC)"
	$(MAKE) all BUILD_TYPE=android CC="$(CC)"

build-android: build-sdl-ttf-android
	@for abi in $(ANDROID_ABIS) ; do \
		case $$abi in \
			"armeabi-v7a") \
				TARGET_TRIPLE="armv7a-linux-androideabi21" ;; \
			"arm64-v8a") \
				TARGET_TRIPLE="aarch64-linux-android21" ;; \
			"x86") \
				TARGET_TRIPLE="i686-linux-android21" ;; \
			"x86_64") \
				TARGET_TRIPLE="x86_64-linux-android21" ;; \
		esac ; \
		$(MAKE) build-android-abi TARGET_TRIPLE=$$TARGET_TRIPLE TARGET_ABI=$$abi TARGET=android/app/src/main/jniLibs/$$abi/libmain.so ; \
	done
	cd android && ./gradlew assembleDebug

# Update clean target
clean-android:
	rm -rf vendor/SDL/build-android-*
	rm -rf vendor/SDL_image/build-android-*
	rm -rf vendor/SDL_ttf/build-android-*
	rm -rf android/app/build
	rm -rf android/app/src/main/jniLibs

help:
	@echo "Available targets:"
	@echo "  make BUILD_TYPE=web	 - Build web version"
	@echo "  make BUILD_TYPE=desktop - Build desktop version"
	@echo "  make clean		  - Clean build directory"
	@echo "  make help			- Show this help message"