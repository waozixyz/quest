#include "pages/routine.h"
#include "rocks.h"
#include "quest_theme.h"

void RenderRoutinePage() {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);

    CLAY_TEXT(CLAY_STRING("Routine Page - Coming Soon"), 
        CLAY_TEXT_CONFIG({ 
            .fontSize = 36,
            .fontId = FONT_ID_TITLE_36,
            .textColor = base_theme.text
        })
    );
}