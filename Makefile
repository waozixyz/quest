# Build configuration
BUILD_TYPE ?= web

INCLUDE_DIR = include
INCLUDE_FLAGS = -I$(INCLUDE_DIR)
BUILD_DIR = build/clay
SRC_DIR = src


ifeq ($(BUILD_TYPE),web)
    # Web target configuration
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
    # Exclude desktop folder for web builds
    SRCS = $(shell find $(SRC_DIR) -name "*.c" ! -path "$(SRC_DIR)/desktop/*")
else ifeq ($(BUILD_TYPE),android)
    # Android target configuration
    NDK_PATH ?= /opt/android-ndk
    SDK_PATH ?= /opt/android-sdk
    
    CC = $(NDK_PATH)/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang
    CFLAGS = -Wall -Werror -O2 -DCLAY_MOBILE
    
    INCLUDE_FLAGS += \
        -I$(NDK_PATH)/sources/android/native_app_glue \
        -I$(NDK_PATH)/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include \
        -I/usr/include/SDL2 -Ivendor/cJSON
    
    LINKER_FLAGS = -shared -lSDL2 -lSDL2_image -lSDL2_ttf -landroid -llog -lm
    
    TARGET = $(BUILD_DIR)/android/jniLibs/arm64-v8a/libmain.so
    # Include all source files plus Android glue
    SRCS = $(shell find $(SRC_DIR) -name "*.c")
    SRCS += $(NDK_PATH)/sources/android/native_app_glue/android_native_app_glue.c
    SRCS += vendor/cJSON/cJSON.c
else
    # Desktop target configuration
    CC = clang
    CFLAGS = -Wall -Werror -O2 -DCLAY_DESKTOP
    LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lm
    INCLUDE_FLAGS += -I/usr/include/SDL2 -Ivendor/cJSON  

    TARGET = $(BUILD_DIR)/game
    # Include all source files for desktop
    SRCS = $(shell find $(SRC_DIR) -name "*.c")
    SRCS += vendor/cJSON/cJSON.c
endif

# Generate object files from sources
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)

# Build and copy assets depending on build type
all: $(TARGET) copy_assets

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LINKER_FLAGS) $(INCLUDE_FLAGS) -o $@ $(OBJS)

# Compile source files
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

# Copy assets based on build type
.PHONY: copy_assets
copy_assets:
ifeq ($(BUILD_TYPE),web)
	cp index.html $(BUILD_DIR)/index.html
	cp manifest.json $(BUILD_DIR)/manifest.json
endif
	cp -r fonts/ $(BUILD_DIR)/fonts
	cp -r images/ $(BUILD_DIR)/images
    
# Clean build directory
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: build-android
build-android:
	# First build the C library
	$(MAKE) BUILD_TYPE=android
	# Then build the APK
	cd android && ./gradlew assembleDebug
    
# Help target
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  make BUILD_TYPE=web    - Build web version"
	@echo "  make BUILD_TYPE=desktop - Build desktop version"
	@echo "  make clean        - Clean build directory"
	@echo "  make help         - Show this help message"