#include "renderers/clay_sdl_renderer_internal.h"

// Global cursor variables 
SDL_Cursor* defaultCursor = NULL;
SDL_Cursor* pointerCursor = NULL;
SDL_Cursor* currentCursor = NULL;

void Clay_SDL2_InitCursors() {
    // Create default arrow cursor
    defaultCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    
    // Create hand pointer cursor
    pointerCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);

    currentCursor = defaultCursor;
    SDL_SetCursor(currentCursor);
}

void Clay_SDL2_CleanupCursors() {
    if (defaultCursor) {
        SDL_FreeCursor(defaultCursor);
        defaultCursor = NULL;
    }
    if (pointerCursor) {
        SDL_FreeCursor(pointerCursor);
        pointerCursor = NULL;
    }
    currentCursor = NULL;
}

SDL_Cursor* Clay_SDL2_GetCurrentCursor() {
    return currentCursor;
}
