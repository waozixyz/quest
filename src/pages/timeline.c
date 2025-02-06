#include "pages/timeline.h"

#include "rocks.h"
#include "quest_theme.h"

void RenderTimelinePage() {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);
    
    CLAY_TEXT(CLAY_STRING("Timeline Page - Coming Soon"), 
        CLAY_TEXT_CONFIG({ 
            .fontSize = 36,
            .fontId = FONT_ID_TITLE_36,
            .textColor = base_theme.text
        })
    );
}
