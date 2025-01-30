#ifndef HABITS_H
#define HABITS_H
#include "rocks.h"
#include "rocks_clay.h"
#include "../state/habits_state.h"
#include "../config.h"
#include "../components/modal.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

// Modal functions
void RenderDeleteHabitModal(void);
void RenderDeleteModalContent(void);

// Habit tab bar functions
void RenderHabitTabBar(void);
void HandleNewTabInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);
void HandleTabInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);
void HandleHabitNameSubmit(const char* text);

void InitializeHabitsPage(Rocks* rocks);

// Cleanup functions
void CleanupHabitsPage(Rocks* rocks);

// Event handling
void HandleHabitsPageInput(InputEvent event);
void ToggleHabitStateForDay(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);

// Main page render function
void RenderHabitsPage(void);

// External state
extern HabitCollection habits;

#endif // HABITS_H