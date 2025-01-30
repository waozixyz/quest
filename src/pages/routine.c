#include "pages/routine.h"
#include "rocks.h"
#include "quest_theme.h"

void RenderRoutinePage() {
    RocksTheme base_theme = rocks_get_theme(g_rocks);

    CLAY_TEXT(CLAY_STRING("Routine Page - Coming Soon"), 
        CLAY_TEXT_CONFIG({ 
            .fontSize = 36,
            .fontId = FONT_ID_TITLE_36,
            .textColor = base_theme.text
        })
    );
}