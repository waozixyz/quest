#include "components/progress_bar.h"

void RenderProgressBar(float completion, Clay_Color color) {
    CLAY(CLAY_LAYOUT({
        .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIXED(20) }
    }),
    CLAY_RECTANGLE({
        .color = COLOR_SECONDARY,
        .cornerRadius = CLAY_CORNER_RADIUS(4)
    })) {
        if (completion > 0.0f) {
            CLAY(CLAY_LAYOUT({
                .sizing = { 
                    CLAY_SIZING_FIXED(completion * 100.0f), 
                    CLAY_SIZING_GROW() 
                }
            }),
            CLAY_RECTANGLE({
                .color = color,
                .cornerRadius = CLAY_CORNER_RADIUS(4)
            })) {}
        }
    }
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

        RenderProgressBar(completion, habit->color);
    }
}
