#ifndef HABITS_STATE_H
#define HABITS_STATE_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "../../vendor/clay/clay.h"

#define MAX_HABIT_STATES 1000

typedef struct {
    uint32_t date;
    uint32_t day_index;
    uint8_t completed;
    uint8_t padding[3];
} HabitState;

typedef struct {
    HabitState states[MAX_HABIT_STATES];
    size_t count;
    Clay_Color color;
} HabitStateCollection;

// Main public functions
void SaveHabitStates(HabitStateCollection* collection);
void LoadHabitStates(HabitStateCollection* collection);
bool ToggleHabitState(HabitStateCollection* collection, uint32_t day_index);
void UpdateHabitColor(HabitStateCollection* collection, Clay_Color color);

#endif