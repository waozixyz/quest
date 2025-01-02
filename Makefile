# Compiler and flags
CC = emcc
WASM_FLAGS = -Wall -Werror -Os -DCLAY_WASM -mbulk-memory --target=wasm32 
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
    -s EXPORTED_FUNCTIONS='["_main", "_printf"]' # Explicitly export main and printf

INCLUDE_FLAGS = -I. -Iinclude

# Directories
BUILD_DIR = build/clay
SRC_DIR = src
INCLUDE_DIR = include

# Find all source files
SRCS = $(shell find $(SRC_DIR) -name "*.c")
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)

# Main target
$(BUILD_DIR)/index.wasm: $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(WASM_FLAGS) $(LINKER_FLAGS) $(INCLUDE_FLAGS) -o $@ $(OBJS)

# Compile source files
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(WASM_FLAGS) $(INCLUDE_FLAGS) -c $< -o $@

# Copy assets
.PHONY: assets
assets:
	cp index.html $(BUILD_DIR)/index.html
	cp -r fonts/ $(BUILD_DIR)/fonts
	cp -r images/ $(BUILD_DIR)/images

# Clean build directory
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

# Default target
.PHONY: all
all: $(BUILD_DIR)/index.wasm assets
