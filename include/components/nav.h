#ifndef NAV_H
#define NAV_H

#include "../clay_extensions.h"

#ifdef __EMSCRIPTEN__
#define CLAY_EXTEND_CONFIG_IMAGE Clay_String sourceURL;
#else
#include <SDL_image.h>
#include <SDL.h>
#endif

#include "../styles.h"
#include "../config.h"
#include "../utils.h"
#include "../../vendor/clay/clay.h"


#ifndef __EMSCRIPTEN__
void InitializeNavIcons(SDL_Renderer* renderer);
void CleanupNavIcons(void);
#endif

void RenderNavigationMenu(void);

#endif
