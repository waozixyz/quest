#include "pages/home.h"

static void RenderQuestDescription() {
    CLAY_TEXT(CLAY_STRING("Quest is a personal life management app focused on habit tracking, task organization, and life visualization tools. Build better habits, organize your tasks, and visualize your life journey all in one place."),
        CLAY_TEXT_CONFIG({
            .fontSize = 14,
            .fontId = FONT_ID_BODY_14,
            .textColor = COLOR_TEXT
        })
    );
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
    CLAY(CLAY_ID("StatsContainer"),
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT() },
            .childGap = 12,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        })
    ) {
        for (size_t i = 0; i < habits.habits_count; i++) {
            float completion = CalculateHabitCompletion(&habits.habits[i]);
            RenderHabitProgressBar(&habits.habits[i], completion);
        }

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

void InitializeHomePage() {
    LoadHabits(&habits);
}

void CleanupHomePage() {
    // Cleanup implementation if needed
}

void HandleHomePageInput(InputEvent event) {
    // Input handling implementation if needed
}