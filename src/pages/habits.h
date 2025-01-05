#ifndef HABITS_H
#define HABITS_H

#include "../../vendor/clay/clay.h"
#include "../state/habits_state.h"
#include "../styles.h"
#include "../events.h"

#include "../components/calendar_box.h"
#include "../components/color_picker.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

void RenderHabitsPage(void);
void ToggleHabitStateForDay(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);

void InitializeHabitsPage(void);
void CleanupHabitsPage(void);
void HandleHabitsPageInput(InputEvent event);


#endif // HABITS_H