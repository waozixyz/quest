#ifndef HABITS_H
#define HABITS_H

#include "../../vendor/clay/clay.h"
#include "../state/habits_state.h"


void RenderHabitsPage(void);
void ToggleHabitStateForDay(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);


#endif // HABITS_H