// clay_sdl_renderer.h
#ifndef CLAY_SDL_RENDERER_H
#define CLAY_SDL_RENDERER_H

#include <SDL.h>
#include <SDL_ttf.h>
#include "../../vendor/clay/clay.h"

void Clay_SDL2_Render(SDL_Renderer *renderer, Clay_RenderCommandArray renderCommands);
bool Clay_SDL2_LoadFont(uint32_t fontId, const char* fontPath, int fontSize);
void Clay_SDL2_InitRenderer(SDL_Renderer *renderer);
void Clay_SDL2_CleanupRenderer(void);


#endif