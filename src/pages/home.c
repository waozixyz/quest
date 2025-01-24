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

void RenderHomePage() {    
    float screenWidth = (float)windowWidth;

    CLAY(CLAY_ID("HomeScrollContainer"), 
        CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .padding = { 32, 32, 32, 32 },
            .childGap = 32,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        }),
        CLAY_SCROLL({ .vertical = true })
    ) {
        // Profile container
        CLAY(CLAY_ID("ProfileContainer"),
            CLAY_LAYOUT({
                .sizing = { 
                    screenWidth < BREAKPOINT_SMALL ? CLAY_SIZING_GROW() : CLAY_SIZING_FIXED(400),
                    CLAY_SIZING_FIXED(400) 
                },
                .padding = { 24, 24, 24, 24 },
                .childGap = 16,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
            }),
            CLAY_RECTANGLE({
                .color = COLOR_PANEL,
                .cornerRadius = CLAY_CORNER_RADIUS(8)
            })
        ) {
            // Stats container
            CLAY(CLAY_ID("StatsContainer"),
                CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT() },
                    .padding = { 16, 16, 16, 16 },
                    .childGap = 12,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM
                })
            ) {
                // Header for stats section
                CLAY_TEXT(CLAY_STRING("Habit Progress"), 
                    CLAY_TEXT_CONFIG({
                        .fontSize = 16,
                        .fontId = FONT_ID_BODY_16,
                        .textColor = COLOR_TEXT
                    })
                );

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
    }
}