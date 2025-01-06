#include "utils.h"
#include "renderers/clay_sdl_renderer.h"
#include <SDL_image.h>
#include <errno.h>

#if defined(CLAY_MOBILE)
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#endif

TTF_Font* loadedFonts[32] = {0};

#if defined(CLAY_MOBILE)
// Define the global Android state
AndroidState g_android_state = {
    .window = NULL,
    .assetManager = NULL,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .initialized = false
};

static AAssetManager* get_asset_manager() {
    pthread_mutex_lock(&g_android_state.mutex);
    void* mgr = g_android_state.assetManager;
    pthread_mutex_unlock(&g_android_state.mutex);
    return (AAssetManager*)mgr;
}
#endif

const char* get_asset_path(const char* filename) {
    static char path[256];
#if defined(CLAY_MOBILE)
    snprintf(path, sizeof(path), "assets/%s", filename);
#else
    strncpy(path, filename, sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';
#endif
    return path;
}

const char* get_font_path(const char* filename) {
    static char path[256];
#if defined(CLAY_MOBILE)
    snprintf(path, sizeof(path), "fonts/%s", filename);
#else
    snprintf(path, sizeof(path), "fonts/%s", filename);
#endif
    return path;
}

const char* get_image_path(const char* filename) {
    static char path[256];
#if defined(CLAY_MOBILE)
    snprintf(path, sizeof(path), "images/%s", filename);
#else
    snprintf(path, sizeof(path), "images/%s", filename);
#endif
    return path;
}

bool Clay_SDL2_LoadFontRW(uint32_t fontId, SDL_RWops* rw, int fontSize) {
    TTF_Font* font = TTF_OpenFontRW(rw, 1, fontSize);  // 1 means SDL_RWops will be auto-closed
    if (!font) {
        SDL_Log("Failed to load font: %s", TTF_GetError());
        return false;
    }

    // Free existing font if any
    if (loadedFonts[fontId]) {
        TTF_CloseFont(loadedFonts[fontId]);
    }

    loadedFonts[fontId] = font;
    return true;
}
bool load_font(uint32_t font_id, const char* filename, int size) {
#if defined(CLAY_MOBILE)
    // Check if SDL_ttf is initialized
    if (!TTF_WasInit()) {
        SDL_Log("SDL_ttf is not initialized before loading font\n");
        return false;
    }

    AAssetManager* mgr = get_asset_manager();
    if (!mgr) {
        SDL_Log("Error: Cannot load font - no asset manager\n");
        return false;
    }

    const char* asset_path = get_font_path(filename);
    SDL_Log("Attempting to load font: %s\n", asset_path);

    AAsset* asset = AAssetManager_open(mgr, asset_path, AASSET_MODE_BUFFER);
    if (!asset) {
        SDL_Log("Error: Cannot open font asset: %s (errno: %d, error: %s)\n", 
                asset_path, errno, strerror(errno));
        return false;
    }

    off_t length = AAsset_getLength(asset);
    void* buffer = malloc(length);
    if (!buffer) {
        SDL_Log("Error: Failed to allocate %ld bytes for font data\n", (long)length);
        AAsset_close(asset);
        return false;
    }

    int read = AAsset_read(asset, buffer, length);
    AAsset_close(asset);

    if (read != length) {
        SDL_Log("Error: Failed to read full font data. Expected %ld bytes, got %d\n", (long)length, read);
        free(buffer);
        return false;
    }

    SDL_RWops* rw = SDL_RWFromMem(buffer, length);
    if (!rw) {
        SDL_Log("Error: Failed to create RWops from memory: %s\n", SDL_GetError());
        free(buffer);
        return false;
    }

    bool success = Clay_SDL2_LoadFontRW(font_id, rw, size);
    free(buffer);
    return success;
#else
    // Non-mobile platform font loading
    return Clay_SDL2_LoadFont(font_id, get_font_path(filename), size);
#endif
}


SDL_Surface* load_image(const char* filename) {
#if defined(CLAY_MOBILE)
    AAssetManager* mgr = get_asset_manager();
    if (!mgr) {
        SDL_Log("Error: Cannot load image - no asset manager\n");
        return NULL;
    }

    const char* asset_path = get_image_path(filename);
    SDL_Log("Attempting to load image: %s\n", asset_path);

    AAsset* asset = AAssetManager_open(mgr, asset_path, AASSET_MODE_BUFFER);
    if (!asset) {
        SDL_Log("Error: Cannot open image asset: %s\n", asset_path);
        return NULL;
    }

    off_t length = AAsset_getLength(asset);
    void* buffer = malloc(length);
    if (!buffer) {
        SDL_Log("Error: Failed to allocate %ld bytes for image data\n", (long)length);
        AAsset_close(asset);
        return NULL;
    }

    int read = AAsset_read(asset, buffer, length);
    AAsset_close(asset);

    if (read != length) {
        SDL_Log("Error: Failed to read full image data\n");
        free(buffer);
        return NULL;
    }

    SDL_RWops* rw = SDL_RWFromMem(buffer, length);
    if (!rw) {
        SDL_Log("Error: Failed to create RWops from memory: %s\n", SDL_GetError());
        free(buffer);
        return NULL;
    }

    SDL_Surface* surface = IMG_Load_RW(rw, 1);  // 1 means free the rwops
    free(buffer);
    return surface;
#else
    return IMG_Load(get_image_path(filename));
#endif
}