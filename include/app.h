#ifndef APP_H
#define APP_H

#include "config.h"

#ifdef __EMSCRIPTEN__
void InitializePages(void);
#else
#include <SDL.h>
#include "platforms/sdl/events.h"

void InitializePages(SDL_Renderer* renderer);
void HandleSDLEvents(bool* running);
void RunGameLoop(SDL_Window* window, SDL_Renderer* renderer);
void HandlePageInput(InputEvent event);
#endif

// void CleanupPages(void);
void RenderCurrentPage(void);
Clay_RenderCommandArray CreateLayout(void);

#endif