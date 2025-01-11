#ifndef __EMSCRIPTEN__

#include "utils.h"
#include "platforms/sdl/renderer.h"
#include <SDL_image.h>
#include <errno.h>
#include <unicode/uchar.h>
#include <unicode/ustring.h>

#if defined(CLAY_MOBILE)
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#endif

// Single global font array, removing the duplicate loadedFonts
SDL2_Font SDL2_fonts[32] = {0};

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
    bool initialized = g_android_state.initialized;
    pthread_mutex_unlock(&g_android_state.mutex);
    
    SDL_Log("get_asset_manager: pointer %p, initialized: %s", 
            mgr, initialized ? "YES" : "NO");
    
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
bool load_font(uint32_t font_id, const char* filename, int size) {
    // Early validation checks
    if (font_id >= 32) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, 
            "Invalid font ID %u (max 31)", font_id);
        return false;
    }

    // Cleanup existing font if any
    if (SDL2_fonts[font_id].font) {
        TTF_CloseFont(SDL2_fonts[font_id].font);
        SDL2_fonts[font_id].font = NULL;
    }

    const char* core_test_strings[] = {
        "abcdefghijklmnopqrstuvwxyz",
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
        "0123456789",
        "!@#$%^&*()_+-=[]{}|;:,.<>?"
    };


    TTF_Font* font = NULL;
    int max_attempts = 3;
    int total_missing_glyphs = 0;
    int total_tested_glyphs = 0;

    for (int attempt = 0; attempt < max_attempts; attempt++) {
        SDL_Log("Font loading attempt %d for %s", attempt + 1, filename);

        // Android-specific loading
        #if defined(CLAY_MOBILE)
        AAssetManager* mgr = get_asset_manager();
        if (!mgr) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "No asset manager available");
            continue;
        }

        const char* asset_path = get_font_path(filename);
        SDL_Log("Attempting to load font from path: %s", asset_path);

        AAsset* asset = AAssetManager_open(mgr, asset_path, AASSET_MODE_BUFFER);
        if (!asset) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, 
                "Failed to open font asset: %s (Error: %s)", 
                asset_path, strerror(errno));
            continue;
        }

        off_t length = AAsset_getLength(asset);
        SDL_Log("Font asset size: %ld bytes", (long)length);

        void* buffer = malloc(length);
        if (!buffer) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, 
                "Failed to allocate %ld bytes for font", (long)length);
            AAsset_close(asset);
            continue;
        }

        int read = AAsset_read(asset, buffer, length);
        AAsset_close(asset);

        if (read != length) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, 
                "Incomplete font read: got %d of %ld bytes", read, (long)length);
            free(buffer);
            continue;
        }

        SDL_RWops* rw = SDL_RWFromMem(buffer, length);
        if (!rw) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, 
                "Failed to create RWops: %s", SDL_GetError());
            free(buffer);
            continue;
        }

        font = TTF_OpenFontRW(rw, 1, size);  // 1 means SDL will free RWops
        #else
        // Non-mobile loading
        const char* font_path = get_font_path(filename);
        font = TTF_OpenFont(font_path, size);
        #endif

        if (!font) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, 
                "Failed to load font: %s", TTF_GetError());
            SDL_Delay(200 * (attempt + 1));  // Exponential backoff
            continue;
        }

        // Enhanced font configuration
        TTF_SetFontHinting(font, TTF_HINTING_LIGHT);
        TTF_SetFontKerning(font, 1);
        TTF_SetFontOutline(font, 0);

        total_missing_glyphs = 0;
        total_tested_glyphs = 0;
        
        for (int i = 0; i < sizeof(core_test_strings)/sizeof(char*); i++) {
            const char* test_string = core_test_strings[i];
            
            for (const char* c = test_string; *c; c++) {
                total_tested_glyphs++;
                
                if (!TTF_GlyphIsProvided(font, (Uint16)*c)) {
                    SDL_Log("Missing glyph for character: '%c' (Unicode: %d)", 
                            *c, (unsigned int)*c);
                    total_missing_glyphs++;
                }
            }
        }

        float missing_percentage = ((float)total_missing_glyphs / total_tested_glyphs) * 100.0f;
        
        SDL_Log("Glyph Coverage Analysis for %s:", filename);
        SDL_Log(" - Total Tested Glyphs: %d", total_tested_glyphs);
        SDL_Log(" - Missing Glyphs: %d", total_missing_glyphs);
        SDL_Log(" - Missing Percentage: %.2f%%", missing_percentage);

        // Stricter glyph coverage requirement
        if (missing_percentage > 10.0f) {  // Allow max 10% missing glyphs
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, 
                "Font %s has insufficient glyph coverage (%.2f%% missing). Retrying.", 
                filename, missing_percentage);
            TTF_CloseFont(font);
            font = NULL;
            continue;
        }

        // If we've made it this far, we've successfully loaded the font
        break;
    }

    // Final validation
    if (!font) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, 
            "Failed to load font %s after multiple attempts", filename);
        return false;
    }

    // Store the font
    SDL2_fonts[font_id].fontId = font_id;
    SDL2_fonts[font_id].font = font;
    
    SDL_Log("Successfully loaded font %s (ID: %u, height: %d)", 
        filename, font_id, TTF_FontHeight(font));
    
    return true;
}

// Add a cleanup function for fonts
void cleanup_fonts() {
    for (int i = 0; i < 32; i++) {
        if (SDL2_fonts[i].font) {
            TTF_CloseFont(SDL2_fonts[i].font);
            SDL2_fonts[i].font = NULL;
            SDL2_fonts[i].fontId = 0;
        }
    }
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

float ScaleUI(float value) {
    return value * globalScalingFactor;
}

int ScaleUIToInt(float value) {
    return (int)(value * globalScalingFactor);
}

SDL_FRect ScaleRect(SDL_Rect rect) {
    return (SDL_FRect){
        rect.x * globalScalingFactor,
        rect.y * globalScalingFactor,
        rect.w * globalScalingFactor,
        rect.h * globalScalingFactor
    };
}

SDL_FRect ScaleRectF(SDL_FRect rect) {
    return (SDL_FRect){
        rect.x * globalScalingFactor,
        rect.y * globalScalingFactor,
        rect.w * globalScalingFactor,
        rect.h * globalScalingFactor
    };
}

#endif