#include "habits.h"
#include "../styles.h"
#include "../components/calendar_box.h"
#include "../components/color_picker.h"
#include <time.h>
#include <emscripten.h>
#include <stdio.h>

HabitStateCollection habit_states = {0};
void ToggleHabitStateForDay(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    // Function works as is since it checks pointerInfo.state
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        uint32_t day_index = (uint32_t)userData;
        ToggleHabitState(&habit_states, day_index);
        
        #ifdef __EMSCRIPTEN__
        SaveHabitStatesCollection(&habit_states);
        #else
        SaveHabitStatesToJSON(&habit_states, "habit_states.json");
        #endif
    }
}

HabitCollection habit_collection = {0};

void HandleColorChange(Clay_Color color, void* user_data) {
    HabitCollection* habits = (HabitCollection*)user_data;
    
    // Ensure we have at least one habit
    if (habits->habit_count == 0) {
        // Initialize first habit if not exists
        habits->habits = malloc(sizeof(HabitDefinition));
        habits->habits_capacity = 1;
        habits->habit_count = 1;
    }
    
    // Update first habit's color
    habits->habits[0].habit_color = color;
    
    // Optional: Save habit collection (implement this function)
    // SaveHabitCollection(habits);
}

void RenderHabitsPage() {
    #ifdef __EMSCRIPTEN__
    LoadHabitStatesCollection(&habit_states);
    #else
    LoadHabitStatesFromJSON(&habit_states, "habit_states.json");
    #endif

    time_t now;
    time(&now);
    struct tm *local_time = localtime(&now);
    
    struct tm today_midnight = *local_time;
    today_midnight.tm_hour = 0;
    today_midnight.tm_min = 0;
    today_midnight.tm_sec = 0;
    time_t today_timestamp = mktime(&today_midnight);

    struct tm start_date = today_midnight;
    int days_to_monday = start_date.tm_wday == 0 ? 6 : start_date.tm_wday - 1;
    start_date.tm_mday -= days_to_monday;
    mktime(&start_date);


    if (habit_collection.habits == NULL) {
        habit_collection.habits = malloc(sizeof(HabitDefinition) * 1);
        habit_collection.habits_capacity = 1;
        habit_collection.habit_count = 1;
        
        // Set initial color
        habit_collection.habits[0].habit_color = COLOR_PRIMARY;
    }

    habit_color_picker = (ColorPickerState){0};


    // Set initial color for the picker
    if (habit_collection.habit_count > 0 && habit_collection.habits != NULL) {
        habit_color_picker.selected_color = habit_collection.habits[0].habit_color;
    } else {
        habit_color_picker.selected_color = COLOR_PRIMARY;
    }

    // Set up callback for color changes
    habit_color_picker.on_color_change = HandleColorChange;
    habit_color_picker.user_data = &habit_collection;

    CLAY(CLAY_ID("HabitsContainer"), 
        CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .padding = { 32, 32 },
            .childGap = 32,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        })
    ) {
        // Title
        CLAY(CLAY_ID("HabitsPageTitle"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) },
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
                .padding = { 0, 32 }
            })
        ) {
            CLAY_TEXT(CLAY_STRING("Habits Calendar"), 
                CLAY_TEXT_CONFIG({ 
                    .fontSize = 48,
                    .fontId = FONT_ID_TITLE_56,
                    .textColor = COLOR_TEXT
                })
            );
        }
        RenderColorPicker(&habit_color_picker);
        CLAY(CLAY_ID("CalendarScrollContainer"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() }
            }),
            CLAY_SCROLL({ .vertical = true })
        ) {
            CLAY(CLAY_ID("CalendarGrid"), 
                CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_FIT(0), CLAY_SIZING_GROW() },
                    .childGap = 32,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .padding = { 16, 16 }
                })
            ) {
                struct tm end_date = start_date;
                end_date.tm_mday += 66;
                mktime(&end_date);
                int days_to_add = (7 - end_date.tm_wday) % 7;
                end_date.tm_mday += days_to_add;
                mktime(&end_date);

                time_t start_time = mktime(&start_date);
                time_t end_time = mktime(&end_date);
                int total_days = (int)((end_time - start_time) / (60 * 60 * 24)) + 1;
                int total_weeks = (total_days + 6) / 7;

                struct tm current = start_date;
                int unique_index = 0;

                for (int row = 0; row < total_weeks; row++) {
                    CLAY(CLAY_IDI("WeekRow", row),
                        CLAY_LAYOUT({
                            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) },
                            .childGap = 10,
                            .layoutDirection = CLAY_LEFT_TO_RIGHT
                        })
                    ) {
                        for (int col = 0; col < 7; col++) {
                            time_t current_timestamp = mktime(&current);
                            bool is_today = (current_timestamp == today_timestamp);
                            bool is_past = (current_timestamp < today_timestamp);
                            
                            // Check if this day is completed
                            bool is_completed = false;
                            for (size_t i = 0; i < habit_states.count; i++) {
                                if (habit_states.states[i].day_index == unique_index && 
                                    habit_states.states[i].completed) {
                                    is_completed = true;
                                    break;
                                }
                            }

                            CalendarBoxProps props = {
                                .day_number = current.tm_mday,
                                .is_today = is_today,
                                .is_past = is_past,
                                .unique_index = unique_index,
                                .is_completed = is_completed,
                                .on_click = ToggleHabitStateForDay,
                                .custom_color = habit_collection.habits[0].habit_color
                            };

                            RenderCalendarBox(props);
                            
                            current.tm_mday++;
                            unique_index++;
                        }
                    }
                }
            }
        }
    }
}