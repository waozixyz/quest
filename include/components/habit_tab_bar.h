#ifndef HABIT_TAB_BAR_H
#define HABIT_TAB_BAR_H

#include "../pages/habits.h"
#include "../config.h"
#include "../utils.h"

typedef struct {
    bool is_open;
    uint32_t habit_id;
    char habit_name[MAX_HABIT_NAME];
} DeleteHabitModal;

void RenderHabitTabBar();
void HandleNewTabInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);
void HandleTabInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);
void HandleHabitNameSubmit(const char* text);

#ifndef __EMSCRIPTEN__
#include "SDL.h"
void InitializeHabitTabBar(SDL_Renderer* renderer);
#endif
void CleanupHabitTabBar(void);
#endif // HABIT_TAB_BAR_H
