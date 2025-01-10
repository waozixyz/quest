#ifndef HABITS_H
#define HABITS_H

#include "../clay_extensions.h"
#include "../../vendor/clay/clay.h"
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

// Platform-specific initialization
#ifndef __EMSCRIPTEN__
#include "SDL.h"
void InitializeHabitTabBar(SDL_Renderer* renderer);
void InitializeHabitsPage(SDL_Renderer* renderer);
#else
void InitializeHabitsPage(void);
#endif

// Cleanup functions
void CleanupHabitTabBar(void);
void CleanupHabitsPage(void);

// Event handling
void HandleHabitsPageInput(InputEvent event);
void ToggleHabitStateForDay(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);

// Main page render function
void RenderHabitsPage(void);

// External state
extern Modal delete_modal;
extern HabitCollection habits;

#endif // HABITS_H