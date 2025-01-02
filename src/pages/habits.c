#include "habits.h"
#include "../styles.h"

void RenderHabitsPage() {
    CLAY_TEXT(CLAY_STRING("Habits Page - Coming Soon"), 
        CLAY_TEXT_CONFIG({ 
            .fontSize = 36,
            .fontId = FONT_ID_TITLE_36,
            .textColor = COLOR_ORANGE
        })
    );
}
