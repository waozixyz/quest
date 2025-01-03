#ifndef HABITS_H
#define HABITS_H

#include "../../vendor/clay/clay.h"
#include "../state/habits_state.h"


typedef struct {
    uint32_t habit_id;
    Clay_Color habit_color;
    // Other habit properties
} HabitDefinition;


typedef struct {
    HabitDefinition* habits;
    size_t habit_count;
    size_t habits_capacity;
} HabitCollection;


void RenderHabitsPage(void);
void ToggleHabitStateForDay(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);


#endif // HABITS_H