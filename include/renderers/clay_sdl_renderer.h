// clay_sdl_renderer.h
#ifndef CLAY_SDL_RENDERER_H
#define CLAY_SDL_RENDERER_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

#include "../clay_extensions.h"
#include "../../vendor/clay/clay.h"

// Cursor management functions
void Clay_SDL2_InitCursors();
void Clay_SDL2_CleanupCursors();
SDL_Cursor* Clay_SDL2_GetCurrentCursor();

// Renderer initialization and cleanup
void Clay_SDL2_InitRenderer(SDL_Renderer *renderer);
void Clay_SDL2_CleanupRenderer(void);

// Font loading
bool Clay_SDL2_LoadFont(uint32_t fontId, const char* fontPath, int fontSize);

// Rendering
void Clay_SDL2_Render(SDL_Renderer *renderer, Clay_RenderCommandArray renderCommands);

#endif // CLAY_SDL_RENDERER_H