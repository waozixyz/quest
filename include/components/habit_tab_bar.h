#ifndef HABIT_TAB_BAR_H
#define HABIT_TAB_BAR_H

#include "../pages/habits.h"
#include "../clay_extensions.h"
#include "../../vendor/clay/clay.h"

void RenderHabitTabBar();
void HandleNewTabInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);
void HandleTabInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);
void HandleHabitNameSubmit(const char* text);

#endif // HABIT_TAB_BAR_H
