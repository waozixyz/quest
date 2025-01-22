#ifndef APP_H
#define APP_H

#include "config.h"
#include "components/nav.h"
#include "pages/home.h"
#include "pages/habits.h"
#include "pages/todos.h"
#include "pages/timeline.h"
#include "pages/routine.h"
#ifndef __EMSCRIPTEN__
#include "platforms/sdl/events.h"
#include "platforms/sdl/renderer.h"
#include "utils.h"

#include <SDL.h>
#endif

#ifdef __EMSCRIPTEN__
void InitializePages(void);

extern void measureTextFunction(Clay_String *text, Clay_TextElementConfig *config);
extern void queryScrollOffsetFunction(Clay_ElementId elementId);
#else
void InitializePages(SDL_Renderer* renderer);
void HandleSDLEvents(bool* running);
void RunGameLoop(SDL_Window* window, SDL_Renderer* renderer);
void HandlePageInput(InputEvent event);
#endif

// void CleanupPages(void);
void RenderCurrentPage(void);
Clay_RenderCommandArray CreateLayout(void);

#endif