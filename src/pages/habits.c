#include "pages/habits.h"

#include "components/calendar_box.h"
#include "components/color_picker.h"
#include "components/date_picker.h"
#include "components/habit_tab_bar.h"

#include "config.h"
HabitCollection habits = {0};
Modal color_picker_modal = {
    .is_open = false,
    .width = 300,
    .height = 300
};

Modal date_picker_modal = {
    .is_open = false,
    .width = 400,
    .height = 500
};


Modal delete_modal = {
    .is_open = false,
    .width = 300,  
    .height = 300 
};

void HandleDateChange(time_t new_date) {
    habits.calendar_start_date = new_date;
    SaveHabits(&habits);
}
#ifdef __EMSCRIPTEN__
void InitializeHabitsPage() {
    LoadHabits(&habits);
    habits.habit_name_input = CreateTextInput(NULL, HandleHabitNameSubmit);
    InitializeDatePicker(habits.calendar_start_date, HandleDateChange, &date_picker_modal);
}

#else
void InitializeHabitsPage(SDL_Renderer* renderer) {
    LoadHabits(&habits);
    habits.habit_name_input = CreateTextInput(NULL, HandleHabitNameSubmit);
    InitializeDatePicker(habits.calendar_start_date, HandleDateChange, &date_picker_modal);
    
    #ifndef __EMSCRIPTEN__
    InitializeHabitTabBar(renderer);
    #endif
}
#endif


void ToggleHabitStateForDay(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        uint32_t day_index = (uint32_t)userData;
        ToggleHabitDay(&habits, day_index);
        SaveHabits(&habits);
    }
}

void HandleColorChange(Clay_Color new_color) {
    UpdateHabitColor(&habits, new_color);
}


void HandleHabitsPageInput(InputEvent event) {
    if (!habits.habit_name_input) return;

    // Only process input if we're editing a new habit
    if (!habits.is_editing_new_habit) return;
    
    // Update text input
    if (event.delta_time > 0) {
        UpdateTextInput(habits.habit_name_input, 0, event.delta_time);
    }

    // Handle text input
    if (event.isTextInput) {
        UpdateTextInput(habits.habit_name_input, event.text[0], event.delta_time);
    } else {
        UpdateTextInput(habits.habit_name_input, event.key, event.delta_time);
    }
}
// Add cleanup for the text input
void CleanupHabitsPage() {
    if (habits.habit_name_input) {
        DestroyTextInput(habits.habit_name_input);
        habits.habit_name_input = NULL;
    }
    CleanupHabitTabBar();
}


void RenderHabitsPage() {
    LoadHabits(&habits);

    Habit* active_habit = GetActiveHabit(&habits);
    if (!active_habit) return;

    time_t now;
    time(&now);

    struct tm today_midnight = {0};  // Initialize to zero
    time_t now_time;
    time(&now_time);
    today_midnight = *localtime(&now_time);
    today_midnight.tm_hour = 0;
    today_midnight.tm_min = 0;
    today_midnight.tm_sec = 0;
    time_t today_timestamp = mktime(&today_midnight);

    struct tm *start_tm = localtime(&habits.calendar_start_date);
    struct tm start_date = *start_tm;

    const int WEEKS_TO_DISPLAY = 10;
    struct tm end_date = start_date;
    end_date.tm_mday += (WEEKS_TO_DISPLAY * 7) - 1;
    mktime(&end_date);

    static const char *day_labels[] = {"S", "M", "T", "W", "T", "F", "S"};

    CLAY(CLAY_ID("HabitsContainer"),
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        })
    ) {
        // Habit tabs
        RenderHabitTabBar();
        
        // Color picker and date picker on the same row
        CLAY(CLAY_ID("ColorAndDatePickerContainer"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) },
                .childGap = 0,  // Set to 0 to remove space between elements
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                .padding = { 0, 0 },  // Remove padding
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }
            })
        ) {
            RenderColorPicker(active_habit->color, HandleColorChange, &color_picker_modal);
            RenderDatePicker(habits.calendar_start_date, HandleDateChange, &date_picker_modal);
            RenderDeleteHabitModal();

        }

        CLAY(CLAY_ID("DayLabels"), 
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) },
                .padding = { 0, 8 },
                .childGap = 8,
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER}
            })
        ) {
            float screenWidth = (float)windowWidth;
            const float MAX_LABEL_WIDTH = 90.0f;  // Maximum label width
            const float MIN_LABEL_WIDTH = 32.0f;  // Minimum label width

            // Calculate label width as a percentage of screen width
            float labelWidth = screenWidth * 0.1f;  // 10% of screen width
            
            // Clamp the label width between min and max
            labelWidth = fmaxf(MIN_LABEL_WIDTH, fminf(labelWidth, MAX_LABEL_WIDTH));

            // Determine font size based on label width
            int labelFontSize = (int)(labelWidth * 0.25f);

            Clay_TextElementConfig *day_label_config = CLAY_TEXT_CONFIG({
                .fontSize = labelFontSize,
                .fontId = FONT_ID_BODY_24,
                .textColor = COLOR_TEXT
            });

            // Rotate day labels based on the start day of the week
            for (int i = 0; i < 7; i++) {
                // Calculate the correct day label index based on the start day of the week
                int label_index = (start_date.tm_wday + i) % 7;

                CLAY(CLAY_IDI("DayLabel", i),
                    CLAY_LAYOUT({
                        .sizing = { 
                            CLAY_SIZING_FIXED(labelWidth), 
                            CLAY_SIZING_FIT(0) 
                        },
                        .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
                    })) { 
                        Clay_String day_str = {
                            .length = strlen(day_labels[label_index]),
                            .chars = day_labels[label_index]
                        };
                        CLAY_TEXT(day_str, day_label_config); 
                    }           
            }
        }

        CLAY(CLAY_ID("CalendarScrollContainer"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) },
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER}
            }),
            CLAY_SCROLL({ .vertical = true })
        ) {
            CLAY(CLAY_ID("CalendarGrid"),
                CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_FIT(0), CLAY_SIZING_FIT(0) },
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


                            char today_str[64];
                            char current_str[64];
                            strftime(today_str, sizeof(today_str), "%m/%d/%y", &today_midnight);
                            strftime(current_str, sizeof(current_str), "%m/%d/%y", &current);

                            bool is_today = (current_timestamp == today_timestamp);
                            bool is_past = (current_timestamp < today_timestamp);

                            // Check if this day is completed in active habit
                            bool is_completed = false;
                            for (size_t i = 0; i < active_habit->days_count; i++) {
                                if (active_habit->calendar_days[i].day_index == unique_index &&
                                    active_habit->calendar_days[i].completed) {
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
                                .custom_color = active_habit->color
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