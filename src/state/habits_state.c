#include <stdio.h>
#include <string.h>
#include <emscripten.h>
#include "habits_state.h"

// JavaScript functions for storage
EM_JS(void, JS_SaveHabitStates, (HabitState* states, int count), {
    try {
        // Get pointer to memory
        const HEAP32 = new Int32Array(Module.HEAP8.buffer);
        const HEAPU8 = new Uint8Array(Module.HEAP8.buffer);
        
        let habitStates = [];
        const stateSize = 12; // size of HabitState struct
        
        console.log('Starting to save states, count:', count);
        console.log('States pointer:', states);
        
        for (let i = 0; i < count; i++) {
            const offset = (states >> 2) + (i * (stateSize >> 2));
            const date = HEAP32[offset];
            const dayIndex = HEAP32[offset + 1];
            const completed = HEAPU8[(offset << 2) + 8] !== 0;
            
            console.log(`Reading state ${i}:`, {offset, date, dayIndex, completed});
            
            habitStates.push({
                date,
                dayIndex,
                completed
            });
        }
        
        const jsonString = JSON.stringify(habitStates);
        console.log('Saving to localStorage:', jsonString);
        localStorage.setItem('habitStates', jsonString);
    } catch (error) {
        console.error('Error saving habit states:', error);
    }
});

EM_JS(int, JS_LoadHabitStates, (HabitState* outStates), {
    try {
        let storedStates = localStorage.getItem('habitStates');
        if (!storedStates) return 0;

        let parsedStates = JSON.parse(storedStates);
        let count = parsedStates.length;
        
        const HEAP32 = new Int32Array(Module.HEAP8.buffer);
        const HEAPU8 = new Uint8Array(Module.HEAP8.buffer);
        const stateSize = 12;

        for (let i = 0; i < count; i++) {
            const offset = (outStates >> 2) + (i * (stateSize >> 2));
            HEAP32[offset] = parsedStates[i].date;
            HEAP32[offset + 1] = parsedStates[i].dayIndex;
            HEAPU8[(offset << 2) + 8] = parsedStates[i].completed ? 1 : 0;
        }

        return count;
    } catch (error) {
        console.error('Error loading habit states:', error);
        return 0;
    }
});
void SaveHabitStatesCollection(HabitStateCollection* collection) {
    printf("Starting to save habit states collection\n");
    
    if (!collection) {  // Remove the states check since it's an array
        printf("Error: Invalid collection pointer\n");
        return;
    }
    
    if (collection->count <= 0 || collection->count > MAX_HABIT_STATES) {
        printf("Error: Invalid count: %zu\n", collection->count);
        return;
    }
    
    printf("Saving %zu states\n", collection->count);
    
    // Print memory layout info
    printf("Size of HabitState: %zu bytes\n", sizeof(HabitState));
    printf("Collection address: %p\n", (void*)collection);
    printf("States array address: %p\n", (void*)collection->states);
    
    // Print each state before saving
    for (size_t i = 0; i < collection->count; i++) {
        printf("State[%zu]: date=%u, day_index=%u, completed=%d\n",
               i,
               collection->states[i].date,
               collection->states[i].day_index,
               collection->states[i].completed);
    }
    
    JS_SaveHabitStates(collection->states, (int)collection->count);
    printf("Save completed\n");
}

void LoadHabitStatesCollection(HabitStateCollection* collection) {
    printf("Starting to load habit states collection\n");
    
    if (!collection) {  // Remove the states check since it's an array
        printf("Error: Invalid collection pointer\n");
        return;
    }
    
    int count = JS_LoadHabitStates(collection->states);
    collection->count = (size_t)count;
    
    printf("Loaded %zu states\n", collection->count);
    
    // Print loaded states
    for (size_t i = 0; i < collection->count; i++) {
        printf("Loaded State[%zu]: date=%u, day_index=%u, completed=%d\n",
               i,
               collection->states[i].date,
               collection->states[i].day_index,
               collection->states[i].completed);
    }
}

bool ToggleHabitState(HabitStateCollection* collection, uint32_t day_index) {
    printf("Toggling habit state for day %u\n", day_index);
    
    // First, check if state already exists and toggle if found
    for (size_t i = 0; i < collection->count; i++) {
        if (collection->states[i].day_index == day_index) {
            // If the state exists, toggle its completion
            collection->states[i].completed = !collection->states[i].completed;
            collection->states[i].date = time(NULL);
            printf("Toggled existing state: completed=%d\n", collection->states[i].completed);
            return true;
        }
    }

    // If no existing state is found and it's not already completed, add a new completed state
    if (collection->count < MAX_HABIT_STATES) {
        HabitState* new_state = &collection->states[collection->count++];
        new_state->day_index = day_index;
        new_state->completed = true;
        new_state->date = time(NULL);
        printf("Added new state: day_index=%u, completed=true\n", day_index);
        return true;
    }

    printf("Error: Cannot add new state, reached MAX_HABIT_STATES\n");
    return false;
}

// File-based storage functions for non-WASM environments
#ifndef __EMSCRIPTEN__
void SaveHabitStatesToJSON(HabitStateCollection* collection, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Error: Could not open file %s for writing\n", filename);
        return;
    }

    fprintf(file, "[\n");
    for (size_t i = 0; i < collection->count; i++) {
        fprintf(file, "  {\n");
        fprintf(file, "    \"date\": %u,\n", collection->states[i].date);
        fprintf(file, "    \"dayIndex\": %u,\n", collection->states[i].day_index);
        fprintf(file, "    \"completed\": %s\n", 
                collection->states[i].completed ? "true" : "false");
        fprintf(file, "  }%s\n", 
                (i < collection->count - 1) ? "," : "");
    }
    fprintf(file, "]\n");
    
    fclose(file);
    printf("Saved habit states to %s\n", filename);
}

void LoadHabitStatesFromJSON(HabitStateCollection* collection, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open file %s for reading\n", filename);
        return;
    }

    collection->count = 0;

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file)) {
        if (strstr(buffer, "\"dayIndex\"")) {
            uint32_t day_index;
            sscanf(buffer, "    \"dayIndex\": %u,", &day_index);
            
            if (collection->count < MAX_HABIT_STATES) {
                HabitState* state = &collection->states[collection->count++];
                state->day_index = day_index;
                state->completed = true;
                state->date = time(NULL);
            }
        }
    }
    
    fclose(file);
    printf("Loaded %zu habit states from %s\n", collection->count, filename);
}
#endif