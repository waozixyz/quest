#include "platforms/sdl/renderer_internal.h"
#include "utils.h"

Clay_Dimensions SDL2_MeasureText(Clay_String *text, Clay_TextElementConfig *config) {    
    // Validate font
    if (config->fontId >= 32) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Invalid fontId: %d", config->fontId);
        return (Clay_Dimensions){0, 0};
    }

    TTF_Font *font = SDL2_fonts[config->fontId].font;
    if (!font) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Font is NULL for fontId: %d", config->fontId);
        return (Clay_Dimensions){0, 0};
    }

    // Early validation of text
    if (!text || !text->chars || text->length == 0) {
        return (Clay_Dimensions){0, (float)TTF_FontHeight(font)};
    }

    // Allocate buffer with proper alignment and checks
    size_t bufferSize = text->length + 1;
    char* chars = (char*)AllocateAligned(8, bufferSize);
    
    if (!chars) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, 
            "Failed to allocate text buffer of size %zu", bufferSize);
        return (Clay_Dimensions){0, 0};
    }

    // Zero initialize buffer and copy text
    memset(chars, 0, bufferSize);
    memcpy(chars, text->chars, text->length);

    #ifdef CLAY_MOBILE
        SDL_Log("Measuring text: '%.*s' (length: %d)",
            (int)text->length, chars, (int)text->length);
    #endif

    int width = 0;
    int height = 0;
    int result = TTF_SizeUTF8(font, chars, &width, &height);
    
    if (result < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, 
            "TTF_SizeUTF8 failed for fontId %d: %s", 
            config->fontId, TTF_GetError());
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, 
            "Problematic text: '%s'", chars);
        free(chars);
        return (Clay_Dimensions){0, 0};
    }

    FreeAligned(chars);
    return (Clay_Dimensions) {
        .width = (float)width,
        .height = (float)height,
    };
}