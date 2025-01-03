#ifndef HABITS_STATE_H
#define HABITS_STATE_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <emscripten.h>

#define MAX_HABIT_STATES 1000


// Ensure consistent memory layout for HabitState struct
#pragma pack(push, 1)
typedef struct {
    uint32_t date;      // 4 bytes
    uint32_t day_index; // 4 bytes
    uint8_t completed;  // 1 byte
    uint8_t padding[3]; // 3 bytes padding for alignment
} HabitState;
#pragma pack(pop)


typedef struct {
    HabitState states[MAX_HABIT_STATES];  // This is an array, not a pointer
    size_t count;
} HabitStateCollection;

// Function prototypes
bool ToggleHabitState(HabitStateCollection* collection, uint32_t day_index);
void SaveHabitStatesToJSON(HabitStateCollection* collection, const char* filename);
void LoadHabitStatesFromJSON(HabitStateCollection* collection, const char* filename);

void SaveHabitStatesInWASM(HabitState* states, int count);
void LoadHabitStatesInWASM(HabitState* states, int* count);


void SaveHabitStatesCollection(HabitStateCollection* collection);
void LoadHabitStatesCollection(HabitStateCollection* collection);


extern void JS_SaveHabitStates(HabitState* states, int count);
extern int JS_LoadHabitStates(HabitState* states);

#endif // HABITS_STATE_H
