# Build configuration
INCLUDE_DIR = include
INCLUDE_FLAGS = -I$(INCLUDE_DIR)
BUILD_DIR = build/clay
SRC_DIR = src
NDK_PATH ?= /opt/android-ndk
SDK_PATH ?= /opt/android-sdk

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
	CC = $(NDK_PATH)/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang
	CFLAGS = -Wall -Werror -O2 -DCLAY_MOBILE -fPIC
	INCLUDE_FLAGS += \
		-I$(NDK_PATH)/sources/android/native_app_glue \
		-I$(NDK_PATH)/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include \
		-I$(NDK_PATH)/sources/android \
		-Ivendor/cJSON \
		-Ivendor/SDL/include \
		-Ivendor/SDL_image/include \
    	-Ivendor/SDL_ttf
		
	LINKER_FLAGS = -shared -landroid -llog -lm \
		-L$(BUILD_DIR)/../android/app/src/main/jniLibs/arm64-v8a \
		-Wl,-rpath-link,$(BUILD_DIR)/../android/app/src/main/jniLibs/arm64-v8a \
		-Landroid/app/src/main/jniLibs/arm64-v8a \
		-lSDL2 -lSDL2_image -lSDL2_ttf
	TARGET = android/app/src/main/jniLibs/arm64-v8a/libmain.so
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
	cd vendor/SDL && \
	mkdir -p build-android && \
	cd build-android && \
	cmake .. \
		-DCMAKE_TOOLCHAIN_FILE=$(NDK_PATH)/build/cmake/android.toolchain.cmake \
		-DANDROID_ABI=arm64-v8a \
		-DANDROID_PLATFORM=android-21 \
		-DBUILD_SHARED_LIBS=ON && \
	make
	mkdir -p android/app/src/main/jniLibs/arm64-v8a/
	cp vendor/SDL/build-android/libSDL2.so android/app/src/main/jniLibs/arm64-v8a/

build-sdl-image-android: build-sdl-android
	cd vendor/SDL_image && \
	mkdir -p build-android && \
	cd build-android && \
	cmake .. \
		-DCMAKE_TOOLCHAIN_FILE=$(NDK_PATH)/build/cmake/android.toolchain.cmake \
		-DANDROID_ABI=arm64-v8a \
		-DANDROID_PLATFORM=android-21 \
		-DSDL2_DIR=../../SDL/build-android \
		-DSDL2_INCLUDE_DIR=../../SDL/include \
		-DSDL2_LIBRARY=../../SDL/build-android/libSDL2.so \
		-DSDL2_PATH=../../SDL \
		-DSDL2IMAGE_AVIF=OFF \
		-DSDL2IMAGE_WEBP=OFF \
		-DSDL2IMAGE_TIFF=OFF && \
	make
	cp vendor/SDL_image/build-android/libSDL2_image.so android/app/src/main/jniLibs/arm64-v8a/

build-sdl-ttf-android: build-sdl-image-android
	cd vendor/SDL_ttf && \
	mkdir -p build-android && \
	cd build-android && \
	cmake .. \
		-DCMAKE_TOOLCHAIN_FILE=$(NDK_PATH)/build/cmake/android.toolchain.cmake \
		-DANDROID_ABI=arm64-v8a \
		-DANDROID_PLATFORM=android-21 \
		-DSDL2_DIR=../../SDL/build-android \
		-DSDL2_INCLUDE_DIR=../../SDL/include \
		-DSDL2_LIBRARY=../../SDL/build-android/libSDL2.so \
		-DSDL2_PATH=../../SDL \
		-DBUILD_SHARED_LIBS=ON \
		-DSDL2TTF_SAMPLES=OFF && \
	make
	cp vendor/SDL_ttf/build-android/libSDL2_ttf.so android/app/src/main/jniLibs/arm64-v8a/

build-android: build-sdl-ttf-android
	$(MAKE) all BUILD_TYPE=android
	cd android && ./gradlew assembleDebug

clean-android:
	rm -rf vendor/SDL/build-android
	rm -rf vendor/SDL_image/build-android
	rm -rf android/app/build
	rm -rf android/app/src/main/jniLibs

help:
	@echo "Available targets:"
	@echo "  make BUILD_TYPE=web	 - Build web version"
	@echo "  make BUILD_TYPE=desktop - Build desktop version"
	@echo "  make clean		  - Clean build directory"
	@echo "  make help			- Show this help message"