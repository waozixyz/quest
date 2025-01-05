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
#include "../../vendor/cJSON/cJSON.h"
#include <stdlib.h>

#define HABITS_FILE "habits.json"

static cJSON* HabitToJSON(const Habit* habit) {
    cJSON* habitObj = cJSON_CreateObject();
    cJSON_AddNumberToObject(habitObj, "id", habit->id);
    cJSON_AddStringToObject(habitObj, "name", habit->name);
    
    cJSON* color = cJSON_CreateObject();
    cJSON_AddNumberToObject(color, "r", habit->color.r);
    cJSON_AddNumberToObject(color, "g", habit->color.g);
    cJSON_AddNumberToObject(color, "b", habit->color.b);
    cJSON_AddNumberToObject(color, "a", habit->color.a);
    cJSON_AddItemToObject(habitObj, "color", color);

    cJSON* days = cJSON_CreateArray();
    for (size_t i = 0; i < habit->days_count; i++) {
        cJSON* day = cJSON_CreateObject();
        cJSON_AddNumberToObject(day, "date", habit->calendar_days[i].date);
        cJSON_AddNumberToObject(day, "day_index", habit->calendar_days[i].day_index);
        cJSON_AddBoolToObject(day, "completed", habit->calendar_days[i].completed);
        cJSON_AddItemToArray(days, day);
    }
    cJSON_AddItemToObject(habitObj, "calendar_days", days);

    return habitObj;
}

static void SaveHabitsJSON(const HabitCollection* collection) {
    cJSON* root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "active_habit_id", collection->active_habit_id);
    
    cJSON* habits = cJSON_CreateArray();
    for (size_t i = 0; i < collection->habits_count; i++) {
        cJSON* habit = HabitToJSON(&collection->habits[i]);
        cJSON_AddItemToArray(habits, habit);
    }
    cJSON_AddItemToObject(root, "habits", habits);

    char* jsonStr = cJSON_Print(root);
    FILE* file = fopen(HABITS_FILE, "w");
    if (file) {
        fputs(jsonStr, file);
        fclose(file);
    }

    free(jsonStr);
    cJSON_Delete(root);
}

static void LoadHabitFromJSON(Habit* habit, cJSON* habitObj) {
    habit->id = cJSON_GetObjectItem(habitObj, "id")->valueint;
    strncpy(habit->name, cJSON_GetObjectItem(habitObj, "name")->valuestring, MAX_HABIT_NAME - 1);
    
    cJSON* color = cJSON_GetObjectItem(habitObj, "color");
    habit->color.r = cJSON_GetObjectItem(color, "r")->valuedouble;
    habit->color.g = cJSON_GetObjectItem(color, "g")->valuedouble;
    habit->color.b = cJSON_GetObjectItem(color, "b")->valuedouble;
    habit->color.a = cJSON_GetObjectItem(color, "a")->valuedouble;

    cJSON* days = cJSON_GetObjectItem(habitObj, "calendar_days");
    habit->days_count = 0;
    cJSON* day;
    cJSON_ArrayForEach(day, days) {
        if (habit->days_count >= MAX_CALENDAR_DAYS) break;
        
        HabitDay* habitDay = &habit->calendar_days[habit->days_count++];
        habitDay->date = cJSON_GetObjectItem(day, "date")->valueint;
        habitDay->day_index = cJSON_GetObjectItem(day, "day_index")->valueint;
        habitDay->completed = cJSON_IsTrue(cJSON_GetObjectItem(day, "completed"));
    }
}

static void CreateDefaultHabitsJSON() {
    HabitCollection defaultCollection = {0};
    Habit* default_habit = &defaultCollection.habits[0];
    strncpy(default_habit->name, "Meditation", MAX_HABIT_NAME - 1);
    default_habit->id = 0;
    default_habit->color = COLOR_PRIMARY;
    default_habit->days_count = 0;
    defaultCollection.habits_count = 1;
    defaultCollection.active_habit_id = 0;

    SaveHabitsJSON(&defaultCollection);
}

static void LoadHabitsJSON(HabitCollection* collection) {
    FILE* file = fopen(HABITS_FILE, "r");
    if (!file) {
        CreateDefaultHabitsJSON();
        file = fopen(HABITS_FILE, "r");
        if (!file) return;
    }

    // Get file size and read entire file
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* jsonStr = malloc(fsize + 1);
    fread(jsonStr, 1, fsize, file);
    jsonStr[fsize] = 0;
    fclose(file);

    cJSON* root = cJSON_Parse(jsonStr);
    free(jsonStr);

    if (!root) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) {
            fprintf(stderr, "Error parsing JSON: %s\n", error_ptr);
        }
        return;
    }

    memset(collection, 0, sizeof(HabitCollection));
    collection->active_habit_id = cJSON_GetObjectItem(root, "active_habit_id")->valueint;

    cJSON* habits = cJSON_GetObjectItem(root, "habits");
    cJSON* habit;
    cJSON_ArrayForEach(habit, habits) {
        if (collection->habits_count >= MAX_HABITS) break;
        LoadHabitFromJSON(&collection->habits[collection->habits_count++], habit);
    }

    cJSON_Delete(root);
}

#endif

void SaveHabits(HabitCollection* collection) {
    if (!collection) return;

    #ifdef __EMSCRIPTEN__
        JS_SaveHabits(collection);
    #else
        SaveHabitsJSON(collection);
    #endif
}

void LoadHabits(HabitCollection* collection) {
    if (!collection) return;


    #ifdef __EMSCRIPTEN__
        JS_LoadHabits(collection);
    #else
        LoadHabitsJSON(collection);
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