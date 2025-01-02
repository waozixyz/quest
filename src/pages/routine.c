#include "routine.h"
#include "../styles.h"

void RenderRoutinePage() {
    CLAY_TEXT(CLAY_STRING("Routine Page - Coming Soon"), 
        CLAY_TEXT_CONFIG({ 
            .fontSize = 36,
            .fontId = FONT_ID_TITLE_36,
            .textColor = COLOR_TEXT
        })
    );
}