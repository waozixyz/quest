#include "components/habit_tab_bar.h"

#ifndef __EMSCRIPTEN__
static SDL_Texture* check_texture = NULL;
static SDL_Texture* edit_texture = NULL;
static SDL_Texture* trash_texture = NULL;

void InitializeHabitTabBar(SDL_Renderer* renderer) {
    SDL_Surface* surface;

    surface = load_image("icons/check.png");
    check_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    surface = load_image("icons/edit.png");
    edit_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    surface = load_image("icons/trash.png");
    trash_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
}
#endif

static uint32_t pending_delete_habit_id = 0;
static char pending_delete_habit_name[MAX_HABIT_NAME] = {0};

static void HandleEditButtonClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        uint32_t habit_id = (uint32_t)userData;
        habits.is_editing_new_habit = true;
        habits.active_habit_id = habit_id;
        
        if (habits.habit_name_input) {
            for (size_t i = 0; i < habits.habits_count; i++) {
                if (habits.habits[i].id == habit_id) {
                    SetTextInputText(habits.habit_name_input, habits.habits[i].name);
                    break;
                }
            }
        }
        #ifdef CLAY_MOBILE
        SDL_StartTextInput();
        #endif
    }
}

static void HandleDeleteButtonClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        uint32_t habit_id = (uint32_t)userData;
        for (size_t i = 0; i < habits.habits_count; i++) {
            if (habits.habits[i].id == habit_id) {
                pending_delete_habit_id = habit_id;
                strncpy(pending_delete_habit_name, habits.habits[i].name, MAX_HABIT_NAME - 1);
                pending_delete_habit_name[MAX_HABIT_NAME - 1] = '\0';
                delete_modal.is_open = true;
                break;
            }
        }
    }
}

static void HandleModalConfirm(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        DeleteHabit(&habits, pending_delete_habit_id);
        delete_modal.is_open = false;
    }
}

static void HandleModalCancel(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        delete_modal.is_open = false;
    }
}
void RenderDeleteModalContent() {
    CLAY(CLAY_ID("DeleteModalContent"),
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .childGap = 24,  // Increase gap
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        })
    ) {
        // Title
        CLAY_TEXT(CLAY_STRING("Delete Habit"), 
            CLAY_TEXT_CONFIG({
                .fontSize = 24,  // Larger font
                .fontId = FONT_ID_BODY_24,
                .textColor = COLOR_TEXT
            })
        );

        // Message
        CLAY_TEXT(CLAY_STRING("Are you sure you want to delete:"),
            CLAY_TEXT_CONFIG({
                .fontSize = 16,
                .fontId = FONT_ID_BODY_16,
                .textColor = COLOR_TEXT
            })
        );

        // Habit name in its own container with different styling
        CLAY(CLAY_LAYOUT({
            .padding = { 16, 16 },
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
        }),
        CLAY_RECTANGLE({
            .color = COLOR_PANEL,
            .cornerRadius = CLAY_CORNER_RADIUS(4)
        })) {
            CLAY_TEXT(CLAY_STRING(pending_delete_habit_name),
                CLAY_TEXT_CONFIG({
                    .fontSize = 18,
                    .fontId = FONT_ID_BODY_16,
                    .textColor = COLOR_TEXT
                })
            );
        }

        // Buttons container
        CLAY(CLAY_LAYOUT({
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
            .childGap = 8,
            .childAlignment = { .x = CLAY_ALIGN_X_RIGHT }
        })) {
            // Cancel button
            CLAY(CLAY_ID("CancelButton"),
                CLAY_LAYOUT({
                    .padding = { 8, 8 },
                    .sizing = { CLAY_SIZING_FIT(0), CLAY_SIZING_FIT(0) }
                }),
                CLAY_RECTANGLE({
                    .color = COLOR_PANEL,
                    .cornerRadius = CLAY_CORNER_RADIUS(4),
                    .cursorPointer = true
                }),
                Clay_OnHover(HandleModalCancel, 0)
            ) {
                CLAY_TEXT(CLAY_STRING("Cancel"),
                    CLAY_TEXT_CONFIG({
                        .fontSize = 16,
                        .fontId = FONT_ID_BODY_16,
                        .textColor = COLOR_TEXT
                    })
                );
            }

            // Delete button
            CLAY(CLAY_ID("ConfirmButton"),
                CLAY_LAYOUT({
                    .padding = { 8, 8 },
                    .sizing = { CLAY_SIZING_FIT(0), CLAY_SIZING_FIT(0) }
                }),
                CLAY_RECTANGLE({
                    .color = COLOR_DANGER,
                    .cornerRadius = CLAY_CORNER_RADIUS(4),
                    .cursorPointer = true
                }),
                Clay_OnHover(HandleModalConfirm, 0)
            ) {
                CLAY_TEXT(CLAY_STRING("Delete"),
                    CLAY_TEXT_CONFIG({
                        .fontSize = 16,
                        .fontId = FONT_ID_BODY_16,
                        .textColor = COLOR_TEXT
                    })
                );
            }
        }
    }
}
void RenderDeleteHabitModal(void) {
    if (!delete_modal.is_open) return;
    RenderModal(&delete_modal, RenderDeleteModalContent); 
}
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
        #ifdef CLAY_MOBILE
        SDL_StopTextInput();  // Hide the soft keyboard
        #endif
    }
}


void HandleNewTabInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        AddNewHabit(&habits);
        habits.is_editing_new_habit = true;
        habits.active_habit_id = habits.habits[habits.habits_count - 1].id;
    }
}


static void RenderHabitTab(const Habit* habit) {
    bool isActive = habits.active_habit_id == habit->id;
    bool isEditing = habits.is_editing_new_habit && isActive;

    int minWidth = isEditing ? 200 : (strlen(habit->name) * 10 + 48); 
    
    CLAY(CLAY_IDI("HabitTab", habit->id),
        CLAY_LAYOUT({
            .padding = { 16, 8 },
            .childGap = 8,
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
            .sizing = { 
                CLAY_SIZING_FIXED(minWidth), // Use fixed width based on content
                CLAY_SIZING_FIXED(48) 
            },
            .layoutDirection = CLAY_LEFT_TO_RIGHT
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
            // Container for input and buttons
            CLAY(CLAY_LAYOUT({
                .childGap = 8,
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) }
            })) {
                // Text input
                CLAY(CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) }
                })) {
                    RenderTextInput(habits.habit_name_input, habit->id);
                }

                // Action buttons container
                CLAY(CLAY_LAYOUT({
                    .childGap = 8,
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }
                })) {
                    // Trash button
                    CLAY(CLAY_IDI("DeleteButton", habit->id),
                        CLAY_LAYOUT({
                            .sizing = { CLAY_SIZING_FIXED(32), CLAY_SIZING_FIXED(32) },
                            .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER }
                        }),
                        CLAY_RECTANGLE({
                            .color = Clay_Hovered() ? COLOR_DANGER : COLOR_PANEL,
                            .cornerRadius = CLAY_CORNER_RADIUS(4),
                            .cursorPointer = true
                        }),
                        Clay_OnHover(HandleDeleteButtonClick, habit->id)
                    ) {
                        #ifdef __EMSCRIPTEN__
                        CLAY(CLAY_LAYOUT({
                            .sizing = { CLAY_SIZING_FIXED(24), CLAY_SIZING_FIXED(24) }
                        }),
                        CLAY_IMAGE({
                            .sourceDimensions = { 24, 24 },
                            .sourceURL = CLAY_STRING("icons/trash.png")
                        })) {}
                        #else
                        CLAY(CLAY_LAYOUT({
                            .sizing = { CLAY_SIZING_FIXED(24), CLAY_SIZING_FIXED(24) }
                        }),
                        CLAY_IMAGE({
                            .sourceDimensions = { 24, 24 },
                            .imageData = trash_texture
                        })) {}
                        #endif
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
            }
        } else {
            // Normal tab content
            CLAY(CLAY_LAYOUT({
                .childGap = 8,
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) }
            })) {
                // Habit name
                CLAY(CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) }
                })) {
                    CLAY_TEXT(CLAY_STRING(habit->name), CLAY_TEXT_CONFIG({
                        .fontSize = 14,
                        .fontId = FONT_ID_BODY_14,
                        .textColor = COLOR_TEXT,
                        .wrapMode = CLAY_TEXT_WRAP_NONE
                    }));
                }
                
                if (isActive) {
                    // Edit button
                    CLAY(CLAY_IDI("EditButton", habit->id),
                        CLAY_LAYOUT({
                            .sizing = { CLAY_SIZING_FIXED(32), CLAY_SIZING_FIXED(32) },
                            .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER }
                        }),
                        CLAY_RECTANGLE({
                            .color = Clay_Hovered() ? COLOR_PRIMARY_HOVER : COLOR_PANEL,
                            .cornerRadius = CLAY_CORNER_RADIUS(4),
                            .cursorPointer = true
                        }),
                        Clay_OnHover(HandleEditButtonClick, habit->id)
                    ) {
                        #ifdef __EMSCRIPTEN__
                        CLAY(CLAY_LAYOUT({
                            .sizing = { CLAY_SIZING_FIXED(24), CLAY_SIZING_FIXED(24) }
                        }),
                        CLAY_IMAGE({
                            .sourceDimensions = { 24, 24 },
                            .sourceURL = CLAY_STRING("icons/edit.png")
                        })) {}
                        #else
                        CLAY(CLAY_LAYOUT({
                            .sizing = { CLAY_SIZING_FIXED(24), CLAY_SIZING_FIXED(24) }
                        }),
                        CLAY_IMAGE({
                            .sourceDimensions = { 24, 24 },
                            .imageData = edit_texture
                        })) {}
                        #endif
                    }
                }
            }
        }
    }
}

void RenderHabitTabBar() {
    CLAY(CLAY_ID("HabitTabsContainer"),
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIXED(90) }
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

void HandleTabInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        uint32_t habit_id = (uint32_t)userData;
        habits.is_editing_new_habit = false;
        habits.active_habit_id = habit_id;
        SaveHabits(&habits);
    }
}

void CleanupHabitTabBar(void) {
    #ifndef __EMSCRIPTEN__
    if (check_texture) {
        SDL_DestroyTexture(check_texture);
        check_texture = NULL;
    }
    if (edit_texture) {
        SDL_DestroyTexture(edit_texture);
        edit_texture = NULL;
    }
    if (trash_texture) {
        SDL_DestroyTexture(trash_texture);
        trash_texture = NULL;
    }
    #endif
}
