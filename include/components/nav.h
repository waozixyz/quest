#ifndef NAV_H
#define NAV_H

#include "../config.h"
#include <string.h>

#include "rocks_clay.h"

#ifndef __EMSCRIPTEN__
#include <SDL_image.h>
#include <SDL.h>
#include "../utils.h"

#endif

#ifndef __EMSCRIPTEN__
void InitializeNavIcons(SDL_Renderer* renderer);
void CleanupNavIcons(void);
#endif

void RenderNavigationMenu(void);

#endif
