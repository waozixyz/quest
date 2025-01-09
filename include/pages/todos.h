#ifndef TODOS_H
#define TODOS_H

#include "../clay_extensions.h"
#include "../../vendor/clay/clay.h"
#include "../components/text_input.h"
#include "../config.h"
#include "../state/todos_state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef CLAY_WASM
#define CLAY_EXTEND_CONFIG_IMAGE Clay_String sourceURL;
#else
#include <SDL_image.h>
#include <SDL.h>
#endif

#ifdef CLAY_WASM
void InitializeTodosPage(void);
#else
void InitializeTodosPage(SDL_Renderer* renderer);
#endif

void RenderTodosPage(void);
void CleanupTodosPage(void);
void HandleTodosPageInput(InputEvent event);

#endif