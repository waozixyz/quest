#include "components/habit_tab_bar.h"

#ifndef __EMSCRIPTEN__
static SDL_Texture* check_texture = NULL;

void InitializeHabitTabBar(SDL_Renderer* renderer) {
    // Load the check texture
    SDL_Surface* surface = load_image("icons/check.png");

    check_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
}
#endif

void HandleHabitNameSubmit(const char* text) {
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


static void HandleConfirmButtonClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        HandleHabitNameSubmit(GetTextInputText(habits.habit_name_input));
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
            .cornerRadius = CLAY_CORNER_RADIUS(5),
            .cursorPointer = true
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
                        .cornerRadius = CLAY_CORNER_RADIUS(4),
                        .cursorPointer = true
                    }),
                    Clay_OnHover(HandleConfirmButtonClick, 0)
                ) {
                    #ifdef __EMSCRIPTEN__
                    CLAY(CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(24), CLAY_SIZING_FIXED(24) }
                    }),
                    CLAY_IMAGE({
                        .sourceDimensions = { 24, 24 },
                        .sourceURL = CLAY_STRING("icons/check.png")
                    })) {}
                    #else
                    CLAY(CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(24), CLAY_SIZING_FIXED(24) }
                    }),
                    CLAY_IMAGE({
                        .sourceDimensions = { 24, 24 },
                        .imageData = check_texture
                    })) {}
                    #endif
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
void RenderHabitTabBar() {
    CLAY(CLAY_ID("HabitTabsContainer"),
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIXED(80) }
        }),
        CLAY_RECTANGLE({ 
            .color = COLOR_SECONDARY
         })
    ) {
        CLAY(CLAY_ID("HabitTabs"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_FIT(), CLAY_SIZING_GROW() },
                .childGap = 16,
                .padding = { 16, 0 },
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT
            }),
            CLAY_SCROLL({ .horizontal = true })
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
                    .cornerRadius = CLAY_CORNER_RADIUS(5),
                    .cursorPointer = true
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
}
void HandleNewTabInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        AddNewHabit(&habits);
        habits.is_editing_new_habit = true;
        habits.active_habit_id = habits.habits[habits.habits_count - 1].id;
    }
}

void HandleTabInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
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

void CleanupHabitTabBar(void) {
    #ifndef __EMSCRIPTEN__
    if (check_texture) {
        SDL_DestroyTexture(check_texture);
        check_texture = NULL;
    }
    #endif
}

