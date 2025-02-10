#include "pages/routine.h"
#include "rocks.h"
#include "quest_theme.h"

void RenderRoutinePage() {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);

    CLAY({
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(),
                .height = CLAY_SIZING_GROW()
            },
            .childAlignment = {
                .x = CLAY_ALIGN_X_CENTER,
                .y = CLAY_ALIGN_Y_CENTER
            }
        }
    }) {
        CLAY_TEXT(CLAY_STRING("Routine Page - Coming Soon"),
            CLAY_TEXT_CONFIG({
                .fontSize = 36,
                .fontId = FONT_ID_TITLE_36,
                .textColor = base_theme.text
            })
        );
    }
}