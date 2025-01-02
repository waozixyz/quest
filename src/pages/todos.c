#include "todos.h"
#include "../styles.h"

void RenderTodosPage() {
    CLAY_TEXT(CLAY_STRING("Todos Page - Coming Soon"),
        CLAY_TEXT_CONFIG({
            .fontSize = 36,
            .fontId = FONT_ID_TITLE_36,
            .textColor = COLOR_ORANGE
        })
    );
}