// habits_state.c
#include <stdio.h>
#include <string.h>
#include "habits_state.h"
#include "../styles.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>


// Declare the JavaScript functions using the correct Emscripten macro syntax
EM_JS(void, JS_SaveHabits, (const HabitCollection* collection), {
    // The actual implementation is in JavaScript
    // This is just a declaration for the C code
});

EM_JS(void, JS_LoadHabits, (HabitCollection* collection), {
    // The actual implementation is in JavaScript
    // This is just a declaration for the C code
});

#else

static void SaveHabitsJSON(const HabitCollection* collection, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return;

    fprintf(file, "{\n  \"active_habit_id\": %u,\n", collection->active_habit_id);
    fprintf(file, "  \"habits\": [\n");
    
    for (size_t i = 0; i < collection->habits_count; i++) {
        const Habit* habit = &collection->habits[i];
        
        fprintf(file, "    {\n");
        fprintf(file, "      \"id\": %u,\n", habit->id);
        fprintf(file, "      \"name\": \"%s\",\n", habit->name);
        fprintf(file, "      \"color\": {\"r\": %f, \"g\": %f, \"b\": %f, \"a\": %f},\n",
            habit->color.r, habit->color.g, habit->color.b, habit->color.a);
        
        fprintf(file, "      \"calendar_days\": [\n");
        for (size_t j = 0; j < habit->days_count; j++) {
            fprintf(file, "        {\"date\": %u, \"day_index\": %u, \"completed\": %s}%s\n",
                habit->calendar_days[j].date,
                habit->calendar_days[j].day_index,
                habit->calendar_days[j].completed ? "true" : "false",
                j < habit->days_count - 1 ? "," : "");
        }
        fprintf(file, "      ]\n");
        fprintf(file, "    }%s\n", i < collection->habits_count - 1 ? "," : "");
    }
    
    fprintf(file, "  ]\n}\n");
    fclose(file);
}

static void LoadHabitsJSON(HabitCollection* collection, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return;

    // Reset collection
    memset(collection, 0, sizeof(HabitCollection));
    
    char buffer[1024];
    Habit* current_habit = NULL;
    
    while (fgets(buffer, sizeof(buffer), file)) {
        if (strstr(buffer, "\"active_habit_id\"")) {
            sscanf(buffer, "  \"active_habit_id\": %u", &collection->active_habit_id);
        }
        else if (strstr(buffer, "\"id\"") && collection->habits_count < MAX_HABITS) {
            current_habit = &collection->habits[collection->habits_count++];
            sscanf(buffer, "      \"id\": %u", &current_habit->id);
        }
        else if (strstr(buffer, "\"name\"") && current_habit) {
            sscanf(buffer, "      \"name\": \"%[^\"]\"", current_habit->name);
        }
        else if (strstr(buffer, "\"color\"") && current_habit) {
            sscanf(buffer, "      \"color\": {\"r\": %f, \"g\": %f, \"b\": %f, \"a\": %f}",
                &current_habit->color.r, &current_habit->color.g,
                &current_habit->color.b, &current_habit->color.a);
        }
        else if (strstr(buffer, "\"date\"") && current_habit && 
                 current_habit->days_count < MAX_CALENDAR_DAYS) {
            HabitDay* day = &current_habit->calendar_days[current_habit->days_count++];
            sscanf(buffer, "        {\"date\": %u, \"day_index\": %u, \"completed\": %*s}",
                &day->date, &day->day_index);
            day->completed = strstr(buffer, "true") != NULL;
        }
    }
    
    fclose(file);
}

#endif

void SaveHabits(HabitCollection* collection) {
    if (!collection) return;

    #ifdef __EMSCRIPTEN__
        JS_SaveHabits(collection);
    #else
        SaveHabitsJSON(collection, "habits.json");
    #endif
}

void LoadHabits(HabitCollection* collection) {
    if (!collection) return;


    #ifdef __EMSCRIPTEN__
        JS_LoadHabits(collection);
    #else
        LoadHabitsJSON(collection, "habits.json");
    #endif

    // Initialize default habit if none exists
    if (collection->habits_count == 0) {
        Habit* default_habit = &collection->habits[0];
        strncpy(default_habit->name, "Meditation", MAX_HABIT_NAME - 1);
        default_habit->id = 0;
        default_habit->color = COLOR_PRIMARY;
        default_habit->days_count = 0;
        collection->habits_count = 1;
        collection->active_habit_id = 0;
    }
}

bool ToggleHabitDay(HabitCollection* collection, uint32_t day_index) {
    if (!collection) return false;
    
    Habit* habit = GetActiveHabit(collection);
    if (!habit) return false;
    
    // Check if day exists and toggle it
    for (size_t i = 0; i < habit->days_count; i++) {
        if (habit->calendar_days[i].day_index == day_index) {
            habit->calendar_days[i].completed = !habit->calendar_days[i].completed;
            habit->calendar_days[i].date = time(NULL);
            return true;
        }
    }

    // Add new day
    if (habit->days_count >= MAX_CALENDAR_DAYS) return false;
    
    HabitDay* new_day = &habit->calendar_days[habit->days_count++];
    new_day->day_index = day_index;
    new_day->completed = true;
    new_day->date = time(NULL);
    return true;
}

void UpdateHabitColor(HabitCollection* collection, Clay_Color color) {
    if (!collection) return;
    
    Habit* habit = GetActiveHabit(collection);
    if (habit) {
        habit->color = color;
        SaveHabits(collection);
    }
}

Habit* GetActiveHabit(HabitCollection* collection) {
    return GetHabitById(collection, collection->active_habit_id);
}

void AddNewHabit(HabitCollection* collection) {
    if (!collection || collection->habits_count >= MAX_HABITS) return;
    
    Habit* new_habit = &collection->habits[collection->habits_count];
    snprintf(new_habit->name, MAX_HABIT_NAME, "Habit %zu", collection->habits_count + 1);
    new_habit->id = collection->habits_count;
    new_habit->color = COLOR_PRIMARY;
    new_habit->days_count = 0;
    
    collection->habits_count++;
    collection->active_habit_id = new_habit->id;
    
    SaveHabits(collection);
}

Habit* GetHabitById(HabitCollection* collection, uint32_t id) {
    if (!collection) return NULL;
    
    for (size_t i = 0; i < collection->habits_count; i++) {
        if (collection->habits[i].id == id) {
            return &collection->habits[i];
        }
    }
    
    return NULL;
}