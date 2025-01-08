#ifndef CLAY_SDL_RENDERER_H
#define CLAY_SDL_RENDERER_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

#include "../clay_extensions.h"
#include "../../vendor/clay/clay.h"

// Initialize and cleanup renderer
void Clay_SDL2_InitRenderer(SDL_Renderer *renderer);
void Clay_SDL2_CleanupRenderer(void);

// Cursor management
SDL_Cursor* Clay_SDL2_GetCurrentCursor(void);
void Clay_SDL2_InitCursors(void);
void Clay_SDL2_CleanupCursors(void);

// Rendering control
void Clay_SDL2_SetRenderScale(float scale);
void Clay_SDL2_Render(SDL_Renderer *renderer, Clay_RenderCommandArray renderCommands);

#endif // CLAY_SDL_RENDERER_H