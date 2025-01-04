#include <stdio.h>
#include <string.h>
#include "habits_state.h"
#include "../styles.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_JS(void, JS_SaveHabitStates, (const HabitState* states, int count, float r, float g, float b, float a), {
    try {
        const HEAP32 = new Int32Array(Module.HEAP8.buffer);
        const HEAPU8 = new Uint8Array(Module.HEAP8.buffer);
        
        let habitData = {
            states: [],
            color: {r, g, b, a}
        };
        
        console.log('Saving color:', {r, g, b, a}); // Debug color values
        
        const stateSize = 12; // size of HabitState struct
        
        for (let i = 0; i < count; i++) {
            const offset = (states >> 2) + (i * (stateSize >> 2));
            habitData.states.push({
                date: HEAP32[offset],
                dayIndex: HEAP32[offset + 1],
                completed: HEAPU8[(offset << 2) + 8] !== 0
            });
        }
        
        const jsonString = JSON.stringify(habitData);
        console.log('Saving habitData:', habitData); // Debug full data
        localStorage.setItem('habitData', jsonString);
    } catch (error) {
        console.error('Error saving habit data:', error);
    }
});

EM_JS(int, JS_LoadHabitStates, (HabitState* outStates, float* colorOut), {
    try {
        const stored = localStorage.getItem('habitData');
        if (!stored) return 0;

        const data = JSON.parse(stored);
        console.log('Loaded data from storage:', data); // Debug loaded data
        
        const states = data.states;
        const count = states.length;
        
        const HEAP32 = new Int32Array(Module.HEAP8.buffer);
        const HEAPU8 = new Uint8Array(Module.HEAP8.buffer);
        const HEAPF32 = new Float32Array(Module.HEAP8.buffer);
        const stateSize = 12;

        // Load states
        for (let i = 0; i < count; i++) {
            const offset = (outStates >> 2) + (i * (stateSize >> 2));
            HEAP32[offset] = states[i].date;
            HEAP32[offset + 1] = states[i].dayIndex;
            HEAPU8[(offset << 2) + 8] = states[i].completed ? 1 : 0;
        }

        // Load color
        if (data.color) {
            const colorOffset = colorOut >> 2;
            HEAPF32[colorOffset] = data.color.r;
            HEAPF32[colorOffset + 1] = data.color.g;
            HEAPF32[colorOffset + 2] = data.color.b;
            HEAPF32[colorOffset + 3] = data.color.a;
            
            console.log('Loading color:', data.color); // Debug color values
            console.log('Color memory offset:', colorOffset);
        }

        return count;
    } catch (error) {
        console.error('Error loading habit data:', error);
        return 0;
    }
});

static void SaveHabitStatesWASM(const HabitStateCollection* collection) {
    printf("Saving color: r=%f, g=%f, b=%f, a=%f\n",
           collection->color.r,
           collection->color.g,
           collection->color.b,
           collection->color.a);
           
    JS_SaveHabitStates(
        collection->states,
        (int)collection->count,
        collection->color.r,
        collection->color.g,
        collection->color.b,
        collection->color.a
    );
}

static void LoadHabitStatesWASM(HabitStateCollection* collection) {
    collection->count = (size_t)JS_LoadHabitStates(collection->states, &collection->color.r);
}

#else

static void SaveHabitStatesJSON(const HabitStateCollection* collection, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return;

    fprintf(file, "{\n  \"color\": {\"r\": %f, \"g\": %f, \"b\": %f, \"a\": %f},\n", 
        collection->color.r, collection->color.g, collection->color.b, collection->color.a);
    
    fprintf(file, "  \"states\": [\n");
    for (size_t i = 0; i < collection->count; i++) {
        fprintf(file, "    {\"date\": %u, \"dayIndex\": %u, \"completed\": %s}%s\n",
            collection->states[i].date,
            collection->states[i].day_index,
            collection->states[i].completed ? "true" : "false",
            i < collection->count - 1 ? "," : "");
    }
    fprintf(file, "  ]\n}\n");
    
    fclose(file);
}

static void LoadHabitStatesJSON(HabitStateCollection* collection, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return;

    // Reset collection
    collection->count = 0;
    collection->color = (Clay_Color){0};

    char buffer[1024];
    bool in_states = false;

    while (fgets(buffer, sizeof(buffer), file)) {
        if (strstr(buffer, "\"color\"")) {
            sscanf(buffer, "  \"color\": {\"r\": %f, \"g\": %f, \"b\": %f, \"a\": %f}",
                &collection->color.r, &collection->color.g, 
                &collection->color.b, &collection->color.a);
        }
        else if (strstr(buffer, "\"dayIndex\"")) {
            if (collection->count < MAX_HABIT_STATES) {
                HabitState* state = &collection->states[collection->count++];
                sscanf(buffer, "    {\"date\": %u, \"dayIndex\": %u, \"completed\": %*s}",
                    &state->date, &state->day_index);
                state->completed = strstr(buffer, "true") != NULL;
            }
        }
    }
    
    fclose(file);
}

#endif

// Public functions
void SaveHabitStates(HabitStateCollection* collection) {
    if (!collection || collection->count > MAX_HABIT_STATES) return;

    #ifdef __EMSCRIPTEN__
        SaveHabitStatesWASM(collection);
    #else
        SaveHabitStatesJSON(collection, "habits.json");
    #endif
}

void LoadHabitStates(HabitStateCollection* collection) {
    if (!collection) return;

    #ifdef __EMSCRIPTEN__
        LoadHabitStatesWASM(collection);
    #else
        LoadHabitStatesJSON(collection, "habits.json");
    #endif

    // Set default color if none loaded
    if (collection->color.a == 0) {
        collection->color = COLOR_PRIMARY;
    }
}

bool ToggleHabitState(HabitStateCollection* collection, uint32_t day_index) {
    if (!collection || collection->count >= MAX_HABIT_STATES) return false;
    
    // Check if state exists and toggle it
    for (size_t i = 0; i < collection->count; i++) {
        if (collection->states[i].day_index == day_index) {
            collection->states[i].completed = !collection->states[i].completed;
            collection->states[i].date = time(NULL);
            return true;
        }
    }

    // Add new state
    HabitState* new_state = &collection->states[collection->count++];
    new_state->day_index = day_index;
    new_state->completed = true;
    new_state->date = time(NULL);
    return true;
}

void UpdateHabitColor(HabitStateCollection* collection, Clay_Color color) {
    if (!collection) return;
    collection->color = color;
    SaveHabitStates(collection);
}