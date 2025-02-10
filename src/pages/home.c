#include "pages/home.h"
#include "rocks.h"
#include "quest_theme.h"

static void RenderQuestDescription() {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);

    CLAY_TEXT(CLAY_STRING("Quest is a personal life management app focused on habit tracking, task organization, and life visualization tools. Build better habits, organize your tasks, and visualize your life journey all in one place."),
        CLAY_TEXT_CONFIG({
            .fontSize = 14,
            .fontId = FONT_ID_BODY_14,
            .textColor = base_theme.text
        })
    );
}

static void RenderSettings() {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);

    CLAY_TEXT(CLAY_STRING("Settings content coming soon..."),
        CLAY_TEXT_CONFIG({
            .fontSize = 14,
            .fontId = FONT_ID_BODY_14,
            .textColor = base_theme.text
        })
    );
}

static void RenderHabitProgress() {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);

    CLAY({
        .id = CLAY_ID("StatsContainer"),
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(),
                .height = CLAY_SIZING_FIT()
            },
            .childGap = 12,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        }
    }) {
        for (size_t i = 0; i < habits.habits_count; i++) {
            float completion = CalculateHabitCompletion(&habits.habits[i]);
            RenderHabitProgressBar(&habits.habits[i], completion);
        }

        if (habits.habits_count == 0) {
            CLAY_TEXT(CLAY_STRING("No habits created yet"), 
                CLAY_TEXT_CONFIG({
                    .fontSize = 14,
                    .fontId = FONT_ID_BODY_14,
                    .textColor = base_theme.secondary
                })
            );
        }
    }
}

float CalculateHabitCompletion(const Habit* habit) {
    if (!habit) return 0.0f;
    
    int completed = 0;
    for (size_t i = 0; i < habit->days_count; i++) {
        if (habit->calendar_days[i].completed) {
            completed++;
        }
    }
    return (float)completed / 66.0f;
}

static void RenderTodayHabitItem(const Habit* habit) {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);

    CLAY({
        .id = CLAY_IDI("TodayHabitItem", habit->id),
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(),
                .height = CLAY_SIZING_FIT()
            },
            .padding = CLAY_PADDING_ALL(8),
            .childGap = 8,
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }
        },
        .backgroundColor = base_theme.secondary,
        .cornerRadius = CLAY_CORNER_RADIUS(4)
    }) {
        Clay_String habit_name = {
            .length = strlen(habit->name),
            .chars = habit->name
        };
        CLAY_TEXT(habit_name, 
            CLAY_TEXT_CONFIG({
                .fontSize = 14,
                .fontId = FONT_ID_BODY_14,
                .textColor = base_theme.text
            })
        );
    }
}

static void RenderTodayHabits() {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);

    CLAY({
        .id = CLAY_ID("TodayHabitsContainer"),
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(),
                .height = CLAY_SIZING_FIT()
            },
            .childGap = 12,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        }
    }) {
        bool has_incomplete_habits = false;
        time_t now = time(NULL);
        
        for (size_t i = 0; i < habits.habits_count; i++) {
            if (!IsHabitCompletedForDate(&habits.habits[i], now)) {
                has_incomplete_habits = true;
                RenderTodayHabitItem(&habits.habits[i]);
            }
        }

        if (!has_incomplete_habits) {
            CLAY_TEXT(CLAY_STRING("All habits completed for today!"), 
                CLAY_TEXT_CONFIG({
                    .fontSize = 14,
                    .fontId = FONT_ID_BODY_14,
                    .textColor = base_theme.secondary
                })
            );
        }
    }
}

void RenderHomePage() {
    static bool quest_minimized = false;
    static bool today_habits_minimized = false;
    static bool progress_minimized = true;
    static bool settings_minimized = true;
    
    CLAY({
        .id = CLAY_ID("HomeOuterContainer"),
        .layout = { 
            .sizing = {
                .width = CLAY_SIZING_GROW(),
                .height = CLAY_SIZING_GROW()
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childAlignment = {
                .x = CLAY_ALIGN_X_CENTER,
                .y = CLAY_ALIGN_Y_CENTER
            }
        }
    }) {
        CLAY({
            .id = CLAY_ID("HomeScrollContainer"),
            .layout = { 
                .sizing = {
                    .width = windowWidth < BREAKPOINT_MEDIUM ? 
                        CLAY_SIZING_GROW() : 
                        CLAY_SIZING_FIXED(BREAKPOINT_MEDIUM),
                    .height = CLAY_SIZING_GROW()
                },
                .padding = CLAY_PADDING_ALL(32),
                .childGap = 16,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
            },
            .scroll = { .vertical = true }
        }) {
            CLAY({
                .id = CLAY_ID("ContentContainer"),
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_GROW(),
                        .height = CLAY_SIZING_GROW()
                    },
                    .padding = { 16, 0, 16, 0 },
                    .childGap = 16,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
                }
            }) {
                RenderCard("Quest", 1, &quest_minimized, RenderQuestDescription);
                RenderCard("Today's Habits", 2, &today_habits_minimized, RenderTodayHabits);  
                RenderCard("Habit Progress", 3, &progress_minimized, RenderHabitProgress);
                RenderCard("Settings", 4, &settings_minimized, RenderSettings);
            }
        }
    }
}

void InitializeHomePage() {
    LoadHabits(&habits);
}

void CleanupHomePage() {
    // Cleanup implementation if needed
}

void HandleHomePageInput(InputEvent event) {
    // Input handling implementation if needed
}