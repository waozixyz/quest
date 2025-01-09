#ifndef HABITS_H
#define HABITS_H

#include "../clay_extensions.h"
#include "../../vendor/clay/clay.h"
#include "../state/habits_state.h"
#include "../config.h"



#include <time.h>
#include <stdio.h>
#include <string.h>

void RenderHabitsPage(void);
void ToggleHabitStateForDay(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);
#ifndef __EMSCRIPTEN__
#include "SDL.h"
void InitializeHabitsPage(SDL_Renderer* renderer);

#else
void InitializeHabitsPage(void);

#endif
void CleanupHabitsPage(void);
void HandleHabitsPageInput(InputEvent event);

extern HabitCollection habits;

#endif // HABITS_H