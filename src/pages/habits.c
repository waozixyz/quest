#include "pages/habits.h"

#include "components/calendar_box.h"
#include "components/color_picker.h"
#include "components/date_picker.h"

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

void HandleDateChange(time_t new_date) {
    habits.calendar_start_date = new_date;
    SaveHabits(&habits);
}


static void HandleHabitNameSubmit(const char* text) {
    if (text[0] != '\0') {
        // Find the active habit and update its name
        for (size_t i = 0; i < habits.habits_count; i++) {
            if (habits.habits[i].id == habits.active_habit_id) {
                strncpy(habits.habits[i].name, text, sizeof(habits.habits[i].name) - 1);
                habits.habits[i].name[sizeof(habits.habits[i].name) - 1] = '\0';
                break;
            }
        }
        habits.is_editing_new_habit = false;
        SaveHabits(&habits);
        ClearTextInput(habits.habit_name_input);
    }
}
void InitializeHabitsPage() {
    LoadHabits(&habits);
    habits.habit_name_input = CreateTextInput(NULL, HandleHabitNameSubmit);
    InitializeDatePicker(habits.calendar_start_date, HandleDateChange, &date_picker_modal);

}

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

static void HandleConfirmButtonClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        HandleHabitNameSubmit(GetTextInputText(habits.habit_name_input));
    }
}

static void HandleNewTabInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        AddNewHabit(&habits);
        habits.is_editing_new_habit = true;
        habits.active_habit_id = habits.habits[habits.habits_count - 1].id;
    }
}

static void HandleTabInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    static uint32_t last_click_id = 0;
    static time_t last_click_time = 0;
    static const float DOUBLE_CLICK_TIME = 0.3f; // 300ms for double click

    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        uint32_t habit_id = (uint32_t)userData;
        time_t current_time = time(NULL);
        
        // Check for double click
        if (habit_id == last_click_id && 
            (difftime(current_time, last_click_time) < DOUBLE_CLICK_TIME)) {
            habits.is_editing_new_habit = true;
            habits.active_habit_id = habit_id;
            if (habits.habit_name_input) {
                // Find the habit and set its name as current input text
                for (size_t i = 0; i < habits.habits_count; i++) {
                    if (habits.habits[i].id == habit_id) {
                        SetTextInputText(habits.habit_name_input, habits.habits[i].name);
                        break;
                    }
                }
            } else {
                printf("Error: habit_name_input is NULL\n");
            }
        
        } else {
            // Single click - just set active
            habits.is_editing_new_habit = false;
            habits.active_habit_id = habit_id;
            SaveHabits(&habits);
        }

        last_click_id = habit_id;
        last_click_time = current_time;
    }
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
static void RenderHabitTab(const Habit* habit) {
    bool isActive = habits.active_habit_id == habit->id;
    bool isEditing = habits.is_editing_new_habit && isActive;

    CLAY(CLAY_IDI("HabitTab", habit->id),
        CLAY_LAYOUT({
            .padding = { 16, 8 },
            .childGap = 8,
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
            .sizing = { CLAY_SIZING_FIT({ .min = 150 }), CLAY_SIZING_FIT(0) } // Add minimum width
        }),
        CLAY_RECTANGLE({
            .color = isActive ? COLOR_PRIMARY :
                     (Clay_Hovered() ? COLOR_PRIMARY_HOVER : COLOR_PANEL),
            .cornerRadius = CLAY_CORNER_RADIUS(5)
        }),
        Clay_OnHover(HandleTabInteraction, habit->id)
    ) {
        if (isEditing) {
            // Container for input and confirm button
            CLAY(CLAY_LAYOUT({
                .childGap = 8,
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) }
            })) {
                // Text input with proper sizing
                CLAY(CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) }
                })) {
                    RenderTextInput(habits.habit_name_input, habit->id);
                }

                // Confirm button
                CLAY(CLAY_IDI("ConfirmHabitName", habit->id),
                    CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(32), CLAY_SIZING_FIXED(32) },
                        .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER }
                    }),
                    CLAY_RECTANGLE({
                        .color = Clay_Hovered() ? COLOR_SUCCESS : COLOR_SECONDARY,
                        .cornerRadius = CLAY_CORNER_RADIUS(4)
                    }),
                    Clay_OnHover(HandleConfirmButtonClick, 0)
                ) {
                    CLAY_TEXT(CLAY_STRING("✓"), CLAY_TEXT_CONFIG({
                        .fontSize = 20,
                        .fontId = FONT_ID_BODY_24,
                        .textColor = COLOR_TEXT
                    }));
                }
            }
        } else {
            CLAY_TEXT(CLAY_STRING(habit->name), CLAY_TEXT_CONFIG({
                .fontSize = 20,
                .fontId = FONT_ID_BODY_24,
                .textColor = COLOR_TEXT
            }));
        }
    }
}

// Add cleanup for the text input
void CleanupHabitsPage() {
    if (habits.habit_name_input) {
        DestroyTextInput(habits.habit_name_input);
        habits.habit_name_input = NULL;
    }
}
static void RenderHabitTabs() {
    CLAY(CLAY_ID("HabitTabs"),
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIXED(60) },
            .childGap = 16,
            .padding = { 16, 0 },
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }
        }),
        CLAY_RECTANGLE({ .color = COLOR_SECONDARY })
    ) {
        // Render existing habit tabs
        for (size_t i = 0; i < habits.habits_count; i++) {
            RenderHabitTab(&habits.habits[i]);
        }

        // Add "+" button for new habit
        CLAY(CLAY_ID("NewHabitTab"),
            CLAY_LAYOUT({ .padding = { 16, 8 } }),
            CLAY_RECTANGLE({
                .color = Clay_Hovered() ? COLOR_PRIMARY_HOVER : COLOR_PANEL,
                .cornerRadius = CLAY_CORNER_RADIUS(5)
            }),
            Clay_OnHover(HandleNewTabInteraction, 0)
        ) {
            CLAY_TEXT(CLAY_STRING("+"), CLAY_TEXT_CONFIG({
                .fontSize = 24,
                .fontId = FONT_ID_BODY_24,
                .textColor = COLOR_TEXT
            }));
        }
    }
}

void RenderHabitsPage() {
    LoadHabits(&habits);

    Habit* active_habit = GetActiveHabit(&habits);
    if (!active_habit) return;

    time_t now;
    time(&now);
    struct tm *local_time = localtime(&now);

    struct tm *start_tm = localtime(&habits.calendar_start_date);
    struct tm start_date = *start_tm;

    struct tm today_midnight = *local_time;
    today_midnight.tm_hour = 0;
    today_midnight.tm_min = 0;
    today_midnight.tm_sec = 0;
    time_t today_timestamp = mktime(&today_midnight);

    const int WEEKS_TO_DISPLAY = 10;
    struct tm end_date = start_date;
    end_date.tm_mday += (WEEKS_TO_DISPLAY * 7) - 1;
    mktime(&end_date);

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

        // Habit tabs
        RenderHabitTabs();
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
        }
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