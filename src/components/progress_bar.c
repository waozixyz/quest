#include "components/progress_bar.h"
#include "rocks.h"
#include "quest_theme.h"

void RenderProgressBar(float completion, Clay_Color color) {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);

    CLAY({
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(),
                .height = CLAY_SIZING_FIXED(20)
            }
        },
        .backgroundColor = base_theme.secondary,
        .cornerRadius = CLAY_CORNER_RADIUS(4)
    }) {
        if (completion > 0.0f) {
            CLAY({
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_FIXED(completion * 100.0f),
                        .height = CLAY_SIZING_GROW()
                    }
                },
                .backgroundColor = color,
                .cornerRadius = CLAY_CORNER_RADIUS(4)
            }) {}
        }
    }
}

void RenderHabitProgressBar(const Habit* habit, float completion) {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);

    CLAY({
        .id = CLAY_IDI("HabitProgress", habit->id),
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(),
                .height = CLAY_SIZING_FIXED(30)
            },
            .childGap = 8,
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }
        }
    }) {
        CLAY({
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_FIXED(100),
                    .height = CLAY_SIZING_FIT()
                }
            }
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

        RenderProgressBar(completion, habit->color);
    }
}