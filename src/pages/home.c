// home.c
#include "pages/home.h"
#include "config.h"
#include "utils.h"



float CalculateHabitCompletion(const Habit* habit) {
    if (!habit) return 0.0f;
    
    int completed = 0;
    for (size_t i = 0; i < habit->days_count; i++) {
        if (habit->calendar_days[i].completed) {
            completed++;
        }
    }
    return (float)completed / 66.0f; // 66 days to form a habit
}

void RenderHabitProgressBar(const Habit* habit, float completion) {
    CLAY(CLAY_IDI("HabitProgress", habit->id),
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIXED(30) },
            .childGap = 8,
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }
        })
    ) {
        // Habit name
        CLAY(CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_FIXED(100), CLAY_SIZING_FIT() }
        })) {
            Clay_String habit_name = {
                .length = strlen(habit->name),
                .chars = habit->name
            };
            CLAY_TEXT(habit_name, 
                CLAY_TEXT_CONFIG({
                    .fontSize = 14,
                    .fontId = FONT_ID_BODY_14,
                    .textColor = COLOR_TEXT
                })
            );
        }

        // Progress bar
        CLAY(CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIXED(20) }
        }),
        CLAY_RECTANGLE({
            .color = COLOR_SECONDARY,
            .cornerRadius = CLAY_CORNER_RADIUS(4)
        })) {
            // Only render progress bar fill if there is progress
            if (completion > 0.0f) {
                CLAY(CLAY_LAYOUT({
                    .sizing = { 
                        CLAY_SIZING_FIXED(completion * 100.0f), 
                        CLAY_SIZING_GROW() 
                    }
                }),
                CLAY_RECTANGLE({
                    .color = habit->color,
                    .cornerRadius = CLAY_CORNER_RADIUS(4)
                })) {}
            }
        }

        // Percentage text
        CLAY(CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_FIXED(50), CLAY_SIZING_FIT() }
        })) {
            char percent_str[8];
            snprintf(percent_str, sizeof(percent_str), "%.0f%%", completion * 100);
            Clay_String percent_text = {
                .length = strlen(percent_str),
                .chars = percent_str
            };
            CLAY_TEXT(percent_text,
                CLAY_TEXT_CONFIG({
                    .fontSize = 14,
                    .fontId = FONT_ID_BODY_14,
                    .textColor = COLOR_TEXT_SECONDARY
                })
            );
        }
    }
}

void InitializeHomePage() {
    // Load habits data if needed
    LoadHabits(&habits);
}

void CleanupHomePage() {
    // Cleanup if needed
}

void HandleHomePageInput(InputEvent event) {
    // Handle any home page specific input
}


static void HandleMinimizeClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        bool* is_minimized = (bool*)userData;
        *is_minimized = !*is_minimized;
    }
}



static void RenderSettings() {
    CLAY_TEXT(CLAY_STRING("Settings content coming soon..."),
        CLAY_TEXT_CONFIG({
            .fontSize = 14,
            .fontId = FONT_ID_BODY_14,
            .textColor = COLOR_TEXT
        })
    );
}
static void RenderHabitProgress() {
    // Stats container
    CLAY(CLAY_ID("StatsContainer"),
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT() },
            .childGap = 12,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        })
    ) {
        // Render habit completion stats
        for (size_t i = 0; i < habits.habits_count; i++) {
            float completion = CalculateHabitCompletion(&habits.habits[i]);
            RenderHabitProgressBar(&habits.habits[i], completion);
        }

        // Show message if no habits exist
        if (habits.habits_count == 0) {
            CLAY_TEXT(CLAY_STRING("No habits created yet"), 
                CLAY_TEXT_CONFIG({
                    .fontSize = 14,
                    .fontId = FONT_ID_BODY_14,
                    .textColor = COLOR_TEXT_SECONDARY
                })
            );
        }
    }
}

void RenderCard(const char* title, int card_id, bool* is_minimized, void (*render_content)()) {
    static const Clay_String MINIMIZE_PLUS = { .length = 1, .chars = "+" };
    static const Clay_String MINIMIZE_MINUS = { .length = 1, .chars = "-" };

    CLAY(CLAY_IDI("Card", card_id),
        CLAY_LAYOUT({
            .sizing = { 
                windowWidth < BREAKPOINT_SMALL ? CLAY_SIZING_GROW() : CLAY_SIZING_FIXED(400),
                CLAY_SIZING_FIT() 
            },
            .childGap = 8,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        }),
        CLAY_RECTANGLE({
            .color = COLOR_PANEL,
            .cornerRadius = CLAY_CORNER_RADIUS(8)
        })
    ) {
        // Card Header
        CLAY(CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIXED(40) },
            .padding = { 8, 8, 8, 8 },
            .childGap = 8,
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }
        }),
        CLAY_RECTANGLE({
            .color = COLOR_SECONDARY,
            .cornerRadius = CLAY_CORNER_RADIUS(4)
        })) {
            // Title
            CLAY(CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT() }
            })) {
                Clay_String title_str = {
                    .length = strlen(title),
                    .chars = title
                };
                CLAY_TEXT(title_str, 
                    CLAY_TEXT_CONFIG({
                        .fontSize = 16,
                        .fontId = FONT_ID_BODY_16,
                        .textColor = COLOR_TEXT
                    })
                );
            }
            
            // Minimize button
            CLAY(CLAY_IDI("MinimizeButton", card_id),
                CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_FIXED(24), CLAY_SIZING_FIXED(24) },
                    .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
                }),
                CLAY_RECTANGLE({
                    .color = Clay_Hovered() ? COLOR_PRIMARY_HOVER : COLOR_PANEL,
                    .cornerRadius = CLAY_CORNER_RADIUS(4),
                    .cursorPointer = true
                }),
                Clay_OnHover(HandleMinimizeClick, (intptr_t)is_minimized)
            ) {
                CLAY(CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_FIT(), CLAY_SIZING_FIT() },
                    .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
                })) {
                    CLAY_TEXT(
                        *is_minimized ? MINIMIZE_PLUS : MINIMIZE_MINUS,
                        CLAY_TEXT_CONFIG({
                            .fontSize = 16,
                            .fontId = FONT_ID_BODY_16,
                            .textColor = COLOR_TEXT
                        })
                    );
                }
            }
        }

        // Card Content
        if (!*is_minimized) {
            CLAY(CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT() },
                .padding = { 16, 16, 16, 16 }
            })) {
                render_content();
            }
        }
    }
}

static void RenderQuestDescription() {
    CLAY_TEXT(CLAY_STRING("Quest is a personal life management app focused on habit tracking, task organization, and life visualization tools. Build better habits, organize your tasks, and visualize your life journey all in one place."),
        CLAY_TEXT_CONFIG({
            .fontSize = 14,
            .fontId = FONT_ID_BODY_14,
            .textColor = COLOR_TEXT
        })
    );
}



void RenderHomePage() {
    static bool quest_minimized = false;
    static bool progress_minimized = false;
    static bool settings_minimized = true;

    CLAY(CLAY_ID("HomeScrollContainer"), 
        CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .padding = { 32, 32, 32, 32 },
            .childGap = 16,
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
        }),
        CLAY_SCROLL({ .vertical = true })
    ) {
        RenderCard("Quest", 1, &quest_minimized, RenderQuestDescription);
        RenderCard("Habit Progress", 2, &progress_minimized, RenderHabitProgress);
        RenderCard("Settings", 3, &settings_minimized, RenderSettings);
    }
}

