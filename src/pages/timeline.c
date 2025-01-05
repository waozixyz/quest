#include "pages/timeline.h"

void RenderTimelinePage() {
    CLAY_TEXT(CLAY_STRING("Timeline Page - Coming Soon"), 
        CLAY_TEXT_CONFIG({ 
            .fontSize = 36,
            .fontId = FONT_ID_TITLE_36,
            .textColor = COLOR_TEXT
        })
    );
}
