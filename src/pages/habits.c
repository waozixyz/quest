#include "pages/habits.h"

#include "components/calendar_box.h"
#include "components/color_picker.h"
#include "components/date_picker.h"

#include "config.h"

#include "utils.h"

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

Modal delete_habit_modal = {
    .is_open = false,
    .width = 300,  
    .height = 300 
};

static Uint32 last_click_time = 0;
static uint32_t last_clicked_habit_id = 0;
static Uint32 lastNewTabTime = 0;
const Uint32 NEW_TAB_DEBOUNCE_MS = 250;


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
        habits.is_editing_new_habit = true;
        
        if (habits.habit_name_input) {
            Habit* active_habit = GetActiveHabit(&habits);
            if (active_habit) {
                SetTextInputText(habits.habit_name_input, active_habit->name);
            }
        }
        #ifdef CLAY_MOBILE
        SDL_StartTextInput();
        #endif
    }
}
static void HandleHeaderTitleClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state != CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        return;
    }

    Habit* active_habit = GetActiveHabit(&habits);
    if (!active_habit) {
        return;
    }

    Uint32 current_time = SDL_GetTicks();
    
    // Check if this is a double click on the same habit
    if (last_clicked_habit_id == active_habit->id && 
        (current_time - last_click_time) <= (
        #if defined(__WIN32__) || defined(__WINGDK__)
            GetDoubleClickTime()
        #elif defined(__OS2__)
            WinQuerySysValue(HWND_DESKTOP, SV_DBLCLKTIME)
        #else
            500  // Default double click time in ms
        #endif
        )) {
        // This is a double click - enter edit mode
        HandleEditButtonClick(elementId, pointerInfo, userData);
    }

    last_click_time = current_time;
    last_clicked_habit_id = active_habit->id;
}


static void HandleDeleteButtonClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        uint32_t habit_id = (uint32_t)userData;
        for (size_t i = 0; i < habits.habits_count; i++) {
            if (habits.habits[i].id == habit_id) {
                // Close keyboard when opening delete modal
                #ifdef CLAY_MOBILE
                SDL_StopTextInput();
                #endif
                
                pending_delete_habit_id = habit_id;
                strncpy(pending_delete_habit_name, habits.habits[i].name, MAX_HABIT_NAME - 1);
                pending_delete_habit_name[MAX_HABIT_NAME - 1] = '\0';
                delete_habit_modal.is_open = true;
                break;
            }
        }
    }
}

static void HandleModalConfirm(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        DeleteHabit(&habits, pending_delete_habit_id);
        habits.is_editing_new_habit = false;  
        delete_habit_modal.is_open = false;
    }
}

static void HandleModalCancel(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        delete_habit_modal.is_open = false;
    }
}

void RenderDeleteModalContent() {
    CLAY(CLAY_ID("DeleteModalContent"),
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .childGap = 24,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        })
    ) {
        CLAY_TEXT(CLAY_STRING("Delete Habit"), 
            CLAY_TEXT_CONFIG({
                .fontSize = 24,
                .fontId = FONT_ID_BODY_24,
                .textColor = COLOR_TEXT
            })
        );

        CLAY_TEXT(CLAY_STRING("Are you sure you want to delete:"),
            CLAY_TEXT_CONFIG({
                .fontSize = 16,
                .fontId = FONT_ID_BODY_16,
                .textColor = COLOR_TEXT
            })
        );

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

        CLAY(CLAY_LAYOUT({
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
            .childGap = 8,
            .childAlignment = { .x = CLAY_ALIGN_X_RIGHT }
        })) {
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
    if (!delete_habit_modal.is_open) return;
    RenderModal(&delete_habit_modal, RenderDeleteModalContent); 
}

void HandleHabitNameSubmit(const char* text) {
    if (text[0] != '\0') {
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
        SDL_StopTextInput();
        #endif
    }
}

void HandleNewTabInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        // Add debounce check
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastNewTabTime < NEW_TAB_DEBOUNCE_MS) {
            SDL_Log("New tab ignored - too soon (delta: %u ms)", 
                    currentTime - lastNewTabTime);
            return;
        }
        lastNewTabTime = currentTime;

        AddNewHabit(&habits);
        habits.is_editing_new_habit = true;
        habits.active_habit_id = habits.habits[habits.habits_count - 1].id;
    }
}

static void RenderHabitTab(const Habit* habit) {
    bool isActive = habits.active_habit_id == habit->id;
    
    CLAY(CLAY_IDI("HabitTab", habit->id),
        CLAY_LAYOUT({
            .padding = { 16, 0 },
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
            .sizing = { 
                CLAY_SIZING_FIT(0),
                CLAY_SIZING_FIXED(32) 
            }
        }),
        CLAY_RECTANGLE({
            .color = isActive ? COLOR_PRIMARY :
                     (Clay_Hovered() ? COLOR_PRIMARY_HOVER : COLOR_PANEL),
            .cornerRadius = CLAY_CORNER_RADIUS(5),
            .cursorPointer = true
        }),
        Clay_OnHover(HandleTabInteraction, habit->id)
    ) {
        CLAY_TEXT(CLAY_STRING(habit->name), CLAY_TEXT_CONFIG({
            .fontSize = 14,
            .fontId = FONT_ID_BODY_14,
            .textColor = COLOR_TEXT,
            .wrapMode = CLAY_TEXT_WRAP_NONE
        }));
    }
}
static void RenderHabitHeader() {
    Habit* active_habit = GetActiveHabit(&habits);
    if (!active_habit) return;

    bool isEditing = habits.is_editing_new_habit;

    CLAY(CLAY_ID("HabitHeader"),
        CLAY_LAYOUT({
            .padding = { 16, 16 },
            .childGap = 16,
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
            .childAlignment = { 
                .x = CLAY_ALIGN_X_CENTER,  // Center horizontally
                .y = CLAY_ALIGN_Y_CENTER 
            },
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) }
        }),
        CLAY_RECTANGLE({
            .color = COLOR_BACKGROUND
        })
    ) {
        if (isEditing) {
            CLAY(CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) },
                .childGap = 8,
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
            })) {
                // Text input
                CLAY(CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_FIXED(200), CLAY_SIZING_FIT(0) }  // Fixed width for input
                })) {
                    RenderTextInput(habits.habit_name_input, active_habit->id);
                }

                // Action buttons
                CLAY(CLAY_LAYOUT({
                    .childGap = 8,
                    .layoutDirection = CLAY_LEFT_TO_RIGHT
                })) {
                    // Delete button
                    CLAY(CLAY_ID("DeleteButton"),
                        CLAY_LAYOUT({
                            .sizing = { CLAY_SIZING_FIXED(32), CLAY_SIZING_FIXED(32) },
                            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }  // Add center alignment
                        }),
                        CLAY_RECTANGLE({
                            .color = Clay_Hovered() ? COLOR_DANGER : COLOR_PANEL,
                            .cornerRadius = CLAY_CORNER_RADIUS(4),
                            .cursorPointer = true
                        }),
                        Clay_OnHover(HandleDeleteButtonClick, active_habit->id)
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
                    CLAY(CLAY_ID("ConfirmButton"),
                        CLAY_LAYOUT({
                            .sizing = { CLAY_SIZING_FIXED(32), CLAY_SIZING_FIXED(32) },
                            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }  // Add center alignment
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
            // Title with double-click handler
            CLAY(CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_FIT(0), CLAY_SIZING_FIT(0) }
            }),
            Clay_OnHover(HandleHeaderTitleClick, 0)
            ) {
                CLAY_TEXT(CLAY_STRING(active_habit->name), CLAY_TEXT_CONFIG({
                    .fontSize = 24,
                    .fontId = FONT_ID_BODY_24,
                    .textColor = COLOR_TEXT
                }));
            }
        }
    }
}

void RenderHabitTabBar() {
    CLAY(CLAY_ID("HabitTabsContainer"),
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIXED(62) },
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
        }),
        CLAY_RECTANGLE({ 
            .color = COLOR_SECONDARY
         })
    ) {
        CLAY(CLAY_ID("HabitTabs"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_FIT(), CLAY_SIZING_GROW() },
                .childGap = 8,
                .padding = { 16, 16 },
                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_LEFT_TO_RIGHT
            }),
            CLAY_SCROLL({ .horizontal = true })
        ) {
            for (size_t i = 0; i < habits.habits_count; i++) {
                RenderHabitTab(&habits.habits[i]);
            }

            CLAY(CLAY_ID("NewHabitTab"),
                CLAY_LAYOUT({ .padding = { 16, 0 } }),
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
    InitializeHabitTabBar(renderer);
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
    
    if (!habits.is_editing_new_habit) return;
    
    if (event.delta_time > 0) {
        UpdateTextInput(habits.habit_name_input, 0, event.delta_time);
    }

    if (event.isTextInput) {
        UpdateTextInput(habits.habit_name_input, event.text[0], event.delta_time);
    } else {
        UpdateTextInput(habits.habit_name_input, event.key, event.delta_time);
    }
}

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

    struct tm today_midnight = {0};
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
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
        })
    ) {
        RenderHabitTabBar();
        
        RenderHabitHeader();
        
        CLAY(CLAY_ID("ColorAndDatePickerContainer"),
            CLAY_LAYOUT({
                .sizing = { 
                    windowWidth > BREAKPOINT_MEDIUM + 40 ? CLAY_SIZING_FIXED(BREAKPOINT_MEDIUM + 40) : CLAY_SIZING_GROW(),
                    CLAY_SIZING_FIT(0) 
                },
                .childGap = 0,
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                .padding = { 8, 0 },
                .childAlignment = { 
                    .x = CLAY_ALIGN_X_CENTER,  // Center the container
                    .y = CLAY_ALIGN_Y_CENTER 
                }
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
            const float MAX_LABEL_WIDTH = 90.0f;
            const float MIN_LABEL_WIDTH = 32.0f;
            float labelWidth = screenWidth * 0.1f;
            labelWidth = fmaxf(MIN_LABEL_WIDTH, fminf(labelWidth, MAX_LABEL_WIDTH));
            int labelFontSize = (int)(labelWidth * 0.25f);

            Clay_TextElementConfig *day_label_config = CLAY_TEXT_CONFIG({
                .fontSize = labelFontSize,
                .fontId = FONT_ID_BODY_24,
                .textColor = COLOR_TEXT
            });

            for (int i = 0; i < 7; i++) {
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
                            bool is_today = (current_timestamp == today_timestamp);
                            bool is_past = (current_timestamp < today_timestamp);

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