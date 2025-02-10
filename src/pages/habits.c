#include "pages/habits.h"
#include "components/calendar_box.h"
#include "components/color_picker.h"
#include "components/date_picker.h"
#include "config.h"
#include "utils.h"
#include "rocks.h"
#include "quest_theme.h"

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

#ifndef __EMSCRIPTEN__
static float last_click_time = 0;
static float last_clicked_habit_id = 0;
static float lastNewTabTime = 0;
const float NEW_TAB_DEBOUNCE_MS = 250;
#endif

typedef struct {
    const char* url;
    Clay_Dimensions dimensions;
} HabitIcon;

static HabitIcon HABIT_ICONS[] = {
    {.url = "images/icons/check.png", .dimensions = {24, 24}},
    {.url = "images/icons/edit.png", .dimensions = {24, 24}},
    {.url = "images/icons/trash.png", .dimensions = {24, 24}},
    {.url = "images/icons/eye-closed.png", .dimensions = {24, 24}} 
};

static void* habit_icon_images[4] = {NULL};

void InitializeHabitIcons(Rocks* rocks) {
    for (int i = 0; i < 4; i++) {
        if (habit_icon_images[i]) {
            Rocks_UnloadImage(rocks, habit_icon_images[i]);
            habit_icon_images[i] = NULL;
        }

        habit_icon_images[i] = Rocks_LoadImage(rocks, HABIT_ICONS[i].url);
        if (!habit_icon_images[i]) {
            fprintf(stderr, "Failed to load habit icon %s\n", HABIT_ICONS[i].url);
            continue;
        }
    }
}

void CleanupHabitIcons(Rocks* rocks) {
    for (int i = 0; i < 4; i++) {
        if (habit_icon_images[i]) {
            Rocks_UnloadImage(rocks, habit_icon_images[i]);
            habit_icon_images[i] = NULL;
        }
    }
}

static uint32_t pending_delete_habit_id = 0;
static char pending_delete_habit_name[MAX_HABIT_NAME] = {0};

static void HandleEditButtonClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        habits.is_editing_new_habit = true;
        
        if (habits.habit_name_input) {
            Habit* active_habit = GetActiveHabit(&habits);
            if (active_habit) {
                Rocks_SetTextInputText(habits.habit_name_input, active_habit->name);
            }
        }
        #ifdef CLAY_MOBILE
        Rocks_StartTextInput();
        #endif
    }
}

void HandleRowCollapseToggle(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state != CLAY_POINTER_DATA_PRESSED_THIS_FRAME) return;

    uint32_t row_index = (uint32_t)userData;
    bool found = false;

    // Check if row is already collapsed
    for (size_t i = 0; i < habits.collapsed_rows_count; i++) {
        if (habits.collapsed_rows[i].row_index == row_index) {
            // Remove from collapsed list (expand)
            for (size_t j = i; j < habits.collapsed_rows_count - 1; j++) {
                habits.collapsed_rows[j] = habits.collapsed_rows[j + 1];
            }
            habits.collapsed_rows_count--;
            found = true;
            break;
        }
    }

    // If not found, add to collapsed list
    if (!found && habits.collapsed_rows_count < MAX_CALENDAR_DAYS) {
        habits.collapsed_rows[habits.collapsed_rows_count].row_index = row_index;
        habits.collapsed_rows[habits.collapsed_rows_count].is_collapsed = true;
        habits.collapsed_rows_count++;
    }

    SaveHabits(&habits);
}

void HandleAddWeekRow(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state != CLAY_POINTER_DATA_PRESSED_THIS_FRAME) return;
    
    habits.weeks_to_display++; 
    SaveHabits(&habits);
}

void HandleHeaderTitleClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state != CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        return;
    }

    Habit* active_habit = GetActiveHabit(&habits);
    if (!active_habit) {
        return;
    }

    #ifndef __EMSCRIPTEN__
    float current_time = Rocks_GetTime(GRocks);
    
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
    #else
    HandleEditButtonClick(elementId, pointerInfo, userData);
    #endif
}

static void HandleDeleteButtonClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        uint32_t habit_id = (uint32_t)userData;
        for (size_t i = 0; i < habits.habits_count; i++) {
            if (habits.habits[i].id == habit_id) {
                // Close keyboard when opening delete modal
                #ifdef CLAY_MOBILE
                Rocks_StopTextInput();
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


void RenderDeleteHabitModal(void) {
    if (!delete_habit_modal.is_open) return;
    RenderModal(&delete_habit_modal, RenderDeleteModalContent); 
}

void HandleHabitNameSubmit(const char* text) {
    if (!text) return;  // Add null check
    
    Habit* active_habit = GetActiveHabit(&habits);
    if (!active_habit) return;

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

        if (habits.habit_name_input) {
            Rocks_ClearTextInput(habits.habit_name_input);
        }

        #ifndef __EMSCRIPTEN__
        Rocks_StopTextInput();
        #endif
    }
}

static void HandleConfirmButtonClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        HandleHabitNameSubmit(Rocks_GetTextInputText(habits.habit_name_input));
        #ifdef CLAY_MOBILE
        Rocks_StopTextInput();
        #endif
    }
}
void HandleNewTabInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        printf("Adding new habit...\n"); // Debug print
        
        AddNewHabit(&habits);
        habits.is_editing_new_habit = true;
        habits.active_habit_id = habits.habits[habits.habits_count - 1].id;
        
        if (habits.habit_name_input) {
            Rocks_SetTextInputText(habits.habit_name_input, "");
            printf("Set text input to empty\n"); // Debug print
        }
        
        SaveHabits(&habits);
        printf("New habit added and saved. Count: %zu\n", habits.habits_count); // Debug print
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

void HandleDateChange(time_t new_date) {
    Habit* active_habit = GetActiveHabit(&habits);
    if (active_habit) {
        active_habit->start_date = new_date;
        SaveHabits(&habits);
    }
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

void RenderDeleteModalContent() {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);
    QuestThemeExtension* theme = (QuestThemeExtension*)base_theme.extension;

    CLAY({
        .id = CLAY_ID("DeleteModalContent"),
        .layout = {
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .childGap = 24,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        }
    }) {
        CLAY_TEXT(CLAY_STRING("Delete Habit"), 
            CLAY_TEXT_CONFIG({
                .fontSize = 24,
                .fontId = FONT_ID_BODY_24,
                .textColor = base_theme.text
            })
        );

        CLAY_TEXT(CLAY_STRING("Are you sure you want to delete:"),
            CLAY_TEXT_CONFIG({
                .fontSize = 16,
                .fontId = FONT_ID_BODY_16,
                .textColor = base_theme.text
            })
        );

        CLAY({
            .layout = {
                .padding = CLAY_PADDING_ALL(16),
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
            },
            .backgroundColor = base_theme.background,
            .cornerRadius = CLAY_CORNER_RADIUS(4)
        }) {
            Clay_String habit_name = {
                .length = strlen(pending_delete_habit_name),
                .chars = pending_delete_habit_name
            };
            CLAY_TEXT(habit_name,
                CLAY_TEXT_CONFIG({
                    .fontSize = 18,
                    .fontId = FONT_ID_BODY_16,
                    .textColor = base_theme.text
                })
            );
        }

        CLAY({
            .layout = {
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                .childGap = 8,
                .childAlignment = { .x = CLAY_ALIGN_X_RIGHT }
            }
        }) {
            CLAY({
                .id = CLAY_ID("CancelButton"),
                .layout = {
                    .padding = CLAY_PADDING_ALL(8),
                    .sizing = { CLAY_SIZING_FIT(0), CLAY_SIZING_FIT(0) }
                },
                .backgroundColor = base_theme.background,
                .cornerRadius = CLAY_CORNER_RADIUS(4)
            }) {
                Clay_OnHover(HandleModalCancel, 0);
                CLAY_TEXT(CLAY_STRING("Cancel"),
                    CLAY_TEXT_CONFIG({
                        .fontSize = 16,
                        .fontId = FONT_ID_BODY_16,
                        .textColor = base_theme.text
                    })
                );
            }

            CLAY({
                .id = CLAY_ID("ConfirmButton"),
                .layout = {
                    .padding = CLAY_PADDING_ALL(8),
                    .sizing = { CLAY_SIZING_FIT(0), CLAY_SIZING_FIT(0) }
                },
                .backgroundColor = theme->danger,
                .cornerRadius = CLAY_CORNER_RADIUS(4)
            }) {
                Clay_OnHover(HandleModalConfirm, 0);
                CLAY_TEXT(CLAY_STRING("Delete"),
                    CLAY_TEXT_CONFIG({
                        .fontSize = 16,
                        .fontId = FONT_ID_BODY_16,
                        .textColor = base_theme.text
                    })
                );
            }
        }
    }
}

void RenderHabitTab(const Habit* habit) {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);
    bool isActive = habits.active_habit_id == habit->id;
    
    CLAY({
        .id = CLAY_IDI("HabitTab", habit->id),
        .layout = {
            .padding = CLAY_PADDING_ALL(16),
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
            .sizing = { 
                .width = CLAY_SIZING_FIT(0),
                .height = CLAY_SIZING_FIXED(32)
            }
        },
        .backgroundColor = isActive ? base_theme.primary : 
                          (Clay_Hovered() ? base_theme.primary_hover : base_theme.background),
        .cornerRadius = CLAY_CORNER_RADIUS(5)
    }) {
        Clay_OnHover(HandleTabInteraction, habit->id);
        
        Clay_String habit_str = {
            .length = strlen(habit->name),
            .chars = habit->name
        };
        CLAY_TEXT(habit_str, CLAY_TEXT_CONFIG({
            .fontSize = 14,
            .fontId = FONT_ID_BODY_14,
            .textColor = base_theme.text,
            .wrapMode = CLAY_TEXT_WRAP_NONE
        }));
    }
}
void RenderHabitHeader() {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);
    QuestThemeExtension* theme = (QuestThemeExtension*)base_theme.extension;

    Habit* active_habit = GetActiveHabit(&habits);
    if (!active_habit) return;

    bool isEditing = habits.is_editing_new_habit;

    CLAY({
        .id = CLAY_ID("HabitHeader"),
        .layout = {
            .padding = CLAY_PADDING_ALL(16),
            .childGap = 16,
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
            .childAlignment = { 
                .x = CLAY_ALIGN_X_CENTER,
                .y = CLAY_ALIGN_Y_CENTER 
            },
            .sizing = {
                .width = CLAY_SIZING_GROW(),
                .height = CLAY_SIZING_FIT(0)
            }
        },
        .backgroundColor = base_theme.background
    }) {
        if (isEditing) {
            CLAY({
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_GROW(),
                        .height = CLAY_SIZING_FIT(0)
                    },
                    .childGap = 8,
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
                }
            }) {
                CLAY({
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_FIXED(200),
                            .height = CLAY_SIZING_FIT(0)
                        }
                    }
                }) {
                    Rocks_RenderTextInput(habits.habit_name_input, active_habit->id);
                }

                CLAY({
                    .layout = {
                        .childGap = 8,
                        .layoutDirection = CLAY_LEFT_TO_RIGHT
                    }
                }) {
                    // Delete button
                    CLAY({
                        .id = CLAY_ID("DeleteButton"),
                        .layout = {
                            .sizing = {
                                .width = CLAY_SIZING_FIXED(32),
                                .height = CLAY_SIZING_FIXED(32)
                            },
                            .childAlignment = {
                                .x = CLAY_ALIGN_X_CENTER,
                                .y = CLAY_ALIGN_Y_CENTER
                            }
                        },
                        .backgroundColor = Clay_Hovered() ? theme->danger : base_theme.background,
                        .cornerRadius = CLAY_CORNER_RADIUS(4)
                    }) {
                        Clay_OnHover(HandleDeleteButtonClick, active_habit->id);
                        
                        CLAY({
                            .layout = {
                                .sizing = {
                                    .width = CLAY_SIZING_FIXED(24),
                                    .height = CLAY_SIZING_FIXED(24)
                                }
                            },
                            .image = {
                                .sourceDimensions = HABIT_ICONS[2].dimensions,
                                .imageData = habit_icon_images[2]
                            }
                        }) {}
                    }

                    // Confirm button
                    CLAY({
                        .id = CLAY_ID("ConfirmButton"),
                        .layout = {
                            .sizing = {
                                .width = CLAY_SIZING_FIXED(32),
                                .height = CLAY_SIZING_FIXED(32)
                            },
                            .childAlignment = {
                                .x = CLAY_ALIGN_X_CENTER,
                                .y = CLAY_ALIGN_Y_CENTER
                            }
                        },
                        .backgroundColor = Clay_Hovered() ? theme->success : base_theme.secondary,
                        .cornerRadius = CLAY_CORNER_RADIUS(4)
                    }) {
                        Clay_OnHover(HandleConfirmButtonClick, 0);
                        
                        CLAY({
                            .layout = {
                                .sizing = {
                                    .width = CLAY_SIZING_FIXED(24),
                                    .height = CLAY_SIZING_FIXED(24)
                                }
                            },
                            .image = {
                                .sourceDimensions = HABIT_ICONS[0].dimensions,
                                .imageData = habit_icon_images[0]
                            }
                        }) {}
                    }
                }
            }
        } else {
            // Title and edit button side by side
            CLAY({
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_GROW(),
                        .height = CLAY_SIZING_FIT(0)
                    },
                    .childGap = 8,
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    .childAlignment = { 
                        .x = CLAY_ALIGN_X_CENTER,
                        .y = CLAY_ALIGN_Y_CENTER
                    }
                }
            }) {
                // Title
                Clay_String active_name = {
                    .length = strlen(active_habit->name),
                    .chars = active_habit->name
                };
                CLAY_TEXT(active_name, CLAY_TEXT_CONFIG({
                    .fontSize = 24,
                    .fontId = FONT_ID_BODY_24,
                    .textColor = base_theme.text
                }));

                // Edit button
                CLAY({
                    .id = CLAY_ID("EditButton"),
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_FIXED(32),
                            .height = CLAY_SIZING_FIXED(32)
                        },
                        .childAlignment = {
                            .x = CLAY_ALIGN_X_CENTER,
                            .y = CLAY_ALIGN_Y_CENTER
                        }
                    },
                    .backgroundColor = Clay_Hovered() ? base_theme.primary_hover : base_theme.background,
                    .cornerRadius = CLAY_CORNER_RADIUS(4)
                }) {
                    Clay_OnHover(HandleEditButtonClick, 0);
                    
                    CLAY({
                        .layout = {
                            .sizing = {
                                .width = CLAY_SIZING_FIXED(24),
                                .height = CLAY_SIZING_FIXED(24)
                            }
                        },
                        .image = {
                            .sourceDimensions = HABIT_ICONS[1].dimensions,
                            .imageData = habit_icon_images[1]
                        }
                    }) {}
                }
            }
        }
    }
}
void RenderHabitTabBar() {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);
    QuestThemeExtension* theme = (QuestThemeExtension*)base_theme.extension;

    CLAY({
        .id = CLAY_ID("HabitTabsContainer"),
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(),
                .height = CLAY_SIZING_FIXED(62)
            },
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
        },
        .backgroundColor = base_theme.secondary
    }) {
        CLAY({
            .id = CLAY_ID("HabitTabs"),
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_FIT(),
                    .height = CLAY_SIZING_GROW()
                },
                .childGap = 8,
                .padding = CLAY_PADDING_ALL(16),
                .childAlignment = { 
                    .x = CLAY_ALIGN_X_CENTER,
                    .y = CLAY_ALIGN_Y_CENTER 
                },
                .layoutDirection = CLAY_LEFT_TO_RIGHT
            },
            .scroll = { .horizontal = true }
        }) {
            for (size_t i = 0; i < habits.habits_count; i++) {
                RenderHabitTab(&habits.habits[i]);
            }

            // New tab button
            CLAY({
                .id = CLAY_ID("NewHabitTab"),
                .layout = {
                    .sizing = { 
                        .width = CLAY_SIZING_FIXED(32),
                        .height = CLAY_SIZING_FIXED(32)
                    },
                    .childAlignment = {
                        .x = CLAY_ALIGN_X_CENTER,
                        .y = CLAY_ALIGN_Y_CENTER
                    }
                },
                .backgroundColor = Clay_Hovered() ? base_theme.primary_hover : base_theme.background,
                .cornerRadius = CLAY_CORNER_RADIUS(5)
            }) {
                Clay_OnHover(HandleNewTabInteraction, 0);
                CLAY_TEXT(CLAY_STRING("+"), CLAY_TEXT_CONFIG({
                    .fontSize = 24,
                    .fontId = FONT_ID_BODY_24,
                    .textColor = base_theme.text
                }));
            }
        }
    }
}

void InitializeHabitsPage(Rocks* rocks) {
    if (!rocks) return;
    printf("Initializing habits page\n");
    
    // Clean up existing
    if (habits.habit_name_input) {
        Rocks_DestroyTextInput(habits.habit_name_input); 
        habits.habit_name_input = NULL;
    }
    
    LoadHabits(&habits);
    printf("Creating text input\n");
    habits.habit_name_input = Rocks_CreateTextInput(NULL, HandleHabitNameSubmit);
    printf("Text input: %p\n", (void*)habits.habit_name_input);

    Habit* active_habit = GetActiveHabit(&habits);
    if (active_habit) {
        InitializeDatePicker(active_habit->start_date, HandleDateChange, &date_picker_modal);
    }

    InitializeHabitIcons(rocks);
}

void CleanupHabitsPage(Rocks* rocks) {
    printf("Cleaning up habits page\n");
    if (habits.habit_name_input) {
        printf("Destroying text input\n");
        Rocks_DestroyTextInput(habits.habit_name_input);
        habits.habit_name_input = NULL;
    }
    CleanupHabitIcons(rocks);
    memset(&habits, 0, sizeof(habits));
}

void RenderHabitsPage(float dt) {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);
    QuestThemeExtension* theme = (QuestThemeExtension*)base_theme.extension;

    LoadHabits(&habits);
    Habit* active_habit = GetActiveHabit(&habits);
    if (!active_habit) return;

    if (habits.habit_name_input && habits.is_editing_new_habit) {
        Rocks_UpdateTextInputFromRocksInput(habits.habit_name_input, GRocks->input, dt);
    }
    
    time_t now;
    time(&now);

    struct tm today_midnight = *localtime(&now);
    today_midnight.tm_hour = 0;
    today_midnight.tm_min = 0;
    today_midnight.tm_sec = 0;
    time_t today_timestamp = mktime(&today_midnight);

    struct tm *start_tm = localtime(&active_habit->start_date);
    struct tm start_date = *start_tm;

    // Initialize weeks_to_display if it's 0
    if (habits.weeks_to_display == 0) {
        habits.weeks_to_display = 10; 
    }


    struct tm end_date = start_date;
    end_date.tm_mday += (habits.weeks_to_display * 7) - 1;  
    mktime(&end_date);

    static const char *day_labels[] = {"S", "M", "T", "W", "T", "F", "S"};

    CLAY({
        .id = CLAY_ID("HabitsContainer"),
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(),
                .height = CLAY_SIZING_GROW()
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
        }
    }) {
        RenderHabitTabBar();
        RenderHabitHeader();
            
        CLAY({
            .id = CLAY_ID("ColorAndDatePickerContainer"),
            .layout = {
                .sizing = { 
                    .width = windowWidth > BREAKPOINT_MEDIUM + 40 ? CLAY_SIZING_FIXED(BREAKPOINT_MEDIUM + 40) : CLAY_SIZING_GROW(),
                    .height = CLAY_SIZING_FIT(0) 
                },
                .childGap = 0,
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                .padding = { 8, 8, 0, 0 },
                .childAlignment = { 
                    .x = CLAY_ALIGN_X_CENTER,
                    .y = CLAY_ALIGN_Y_CENTER 
                }
            }
        }) {
            RenderColorPicker(active_habit->color, HandleColorChange, &color_picker_modal);
            RenderDatePicker(active_habit->start_date, HandleDateChange, &date_picker_modal);
            RenderDeleteHabitModal();
        }

        CLAY({
            .id = CLAY_ID("DayLabels"),
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(),
                    .height = CLAY_SIZING_FIT(0)
                },
                .padding = { 0, 0, 8, 8 },
                .childGap = 8,
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
            }
        }) {
            float screenWidth = (float)windowWidth;
            const float MAX_LABEL_WIDTH = 90.0f;
            const float MIN_LABEL_WIDTH = 32.0f;
            float labelWidth = screenWidth * 0.1f;
            labelWidth = fmaxf(MIN_LABEL_WIDTH, fminf(labelWidth, MAX_LABEL_WIDTH));
            int labelFontSize = (int)(labelWidth * 0.25f);

            Clay_TextElementConfig *day_label_config = CLAY_TEXT_CONFIG({
                .fontSize = labelFontSize,
                .fontId = FONT_ID_BODY_24,
                .textColor = base_theme.text
            });

            for (int i = 0; i < 7; i++) {
                int label_index = (start_date.tm_wday + i) % 7;

                CLAY({
                    .id = CLAY_IDI("DayLabel", i),
                    .layout = {
                        .sizing = { 
                            .width = CLAY_SIZING_FIXED(labelWidth),
                            .height = CLAY_SIZING_FIT(0)
                        },
                        .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
                    }
                }) {
                    Clay_String day_str = {
                        .length = strlen(day_labels[label_index]),
                        .chars = day_labels[label_index]
                    };
                    CLAY_TEXT(day_str, day_label_config);
                }
            }
        }

        CLAY({
            .id = CLAY_ID("CalendarScrollContainer"),
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(),
                    .height = CLAY_SIZING_FIT(0)
                },
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
            },
            .scroll = { .vertical = true }
        }) {
            CLAY({
                .id = CLAY_ID("CalendarGrid"),
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_FIT(0),
                        .height = CLAY_SIZING_FIT(0)
                    },
                    .childGap = 32,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .padding = CLAY_PADDING_ALL(16)
                }
            }) {
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

                current.tm_hour = 0;
                current.tm_min = 0;
                current.tm_sec = 0;

                int unique_index = 0;

                // Find today's row for initial scroll position
                int today_row = -1;
                struct tm today = *localtime(&now);
                for (int row = 0; row < total_weeks; row++) {
                    struct tm week_start = start_date;
                    week_start.tm_mday += (row * 7);
                    mktime(&week_start);
                    
                    if (week_start.tm_year == today.tm_year && 
                        week_start.tm_mon == today.tm_mon &&
                        abs(week_start.tm_mday - today.tm_mday) < 7) {
                        today_row = row;
                        break;
                    }
                }

                for (int row = 0; row < total_weeks; row++) {
                    bool is_collapsed = false;
                    for (size_t i = 0; i < habits.collapsed_rows_count; i++) {
                        if (habits.collapsed_rows[i].row_index == row) {
                            is_collapsed = true;
                            break;
                        }
                    }

                    if (is_collapsed) {
                        // When collapsing, skip 7 days
                        current.tm_mday += 7;
                        unique_index += 7;
                        mktime(&current);  // Important: normalize the date after adding days

                        // Render collapsed row
                        CLAY({
                            .id = CLAY_IDI("WeekRowCollapsed", row),
                            .layout = {
                                .sizing = {
                                    .width = CLAY_SIZING_GROW(),
                                    .height = CLAY_SIZING_FIXED(4)
                                },
                                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }
                            },
                            .backgroundColor = base_theme.secondary,
                            .cornerRadius = CLAY_CORNER_RADIUS(2)
                        }) {
                            Clay_OnHover(HandleRowCollapseToggle, row);
                        }
        
                    } else {
                        // Existing week row rendering
                        CLAY({
                            .id = CLAY_IDI("WeekRow", row),
                            .layout = {
                                .sizing = {
                                    .width = CLAY_SIZING_GROW(),
                                    .height = CLAY_SIZING_FIT(0)
                                },
                                .childGap = 10,
                                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                                .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }
                            }
                        }) {
                            // Add collapse button
                            CLAY({
                                .id = CLAY_IDI("CollapseButton", row),
                                .layout = {
                                    .sizing = {
                                        .width = CLAY_SIZING_FIXED(24),
                                        .height = CLAY_SIZING_FIXED(24)
                                    }
                                },
                                .backgroundColor = Clay_Hovered() ? base_theme.primary_hover : base_theme.background,
                                .cornerRadius = CLAY_CORNER_RADIUS(4)
                            }) {
                                Clay_OnHover(HandleRowCollapseToggle, row);
                                CLAY({
                                    .layout = {
                                        .sizing = {
                                            .width = CLAY_SIZING_FIXED(16),
                                            .height = CLAY_SIZING_FIXED(16)
                                        }
                                    },
                                    .image = {
                                        .sourceDimensions = HABIT_ICONS[3].dimensions,
                                        .imageData = habit_icon_images[3]
                                    }
                                }) {}
                            }
                        
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
                    if (row == today_row) {
                        CLAY({
                            .id = CLAY_ID("TodayRowMarker"),
                            .floating = {
                                .attachTo = CLAY_ATTACH_TO_PARENT,
                                .attachPoints = {
                                    .element = CLAY_ATTACH_POINT_CENTER_CENTER,
                                    .parent = CLAY_ATTACH_POINT_CENTER_CENTER
                                }
                            }
                        }) {}
                    }
                }
                
                    // Add week row controls at bottom
                    CLAY({
                        .id = CLAY_ID("WeekControls"),
                        .layout = {
                            .sizing = {
                                .width = CLAY_SIZING_GROW(),
                                .height = CLAY_SIZING_FIT(0)
                            },
                            .childGap = 8,
                            .layoutDirection = CLAY_LEFT_TO_RIGHT,
                            .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
                        }
                    }) {
                        CLAY({
                            .id = CLAY_ID("AddWeekButton"),
                            .layout = {
                                .sizing = {
                                    .width = CLAY_SIZING_FIT(0),
                                    .height = CLAY_SIZING_FIT(0)
                                },
                                .padding = CLAY_PADDING_ALL(8)
                            },
                            .backgroundColor = Clay_Hovered() ? base_theme.primary_hover : base_theme.background,
                            .cornerRadius = CLAY_CORNER_RADIUS(4)
                        }) {
                            Clay_OnHover(HandleAddWeekRow, 0);
                            CLAY_TEXT(CLAY_STRING("Add Week"), CLAY_TEXT_CONFIG({
                                .fontSize = 16,
                                .fontId = FONT_ID_BODY_16,
                                .textColor = base_theme.text
                            }));
                        }
                    }
            }
        }
    }
}