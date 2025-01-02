#include "nav.h"

void RenderNavItem(const char* text, uint32_t pageId) {
    bool isActive = ACTIVE_PAGE == pageId;
    
    CLAY(CLAY_IDI("Nav", pageId), 
        CLAY_LAYOUT({ .padding = { 16, 8 } }), 
        CLAY_RECTANGLE({ 
            .color = isActive ? COLOR_RED_HOVER : 
                     (Clay_Hovered() ? COLOR_LIGHT_HOVER : COLOR_LIGHT),
            .cornerRadius = CLAY_CORNER_RADIUS(5),
            .cursorPointer = true
        }),
        Clay_OnHover(HandleNavInteraction, pageId)
    ) {
        switch(pageId) {
            case 0: CLAY_TEXT(CLAY_STRING("Home"), CLAY_TEXT_CONFIG({ 
                .fontSize = 20,
                .fontId = FONT_ID_BODY_24,
                .textColor = COLOR_ORANGE,
                .disablePointerEvents = true 
            })); break;
            case 1: CLAY_TEXT(CLAY_STRING("Habits"), CLAY_TEXT_CONFIG({ 
                .fontSize = 20,
                .fontId = FONT_ID_BODY_24,
                .textColor = COLOR_ORANGE,
                .disablePointerEvents = true 
            })); break;
            case 2: CLAY_TEXT(CLAY_STRING("Todos"), CLAY_TEXT_CONFIG({ 
                .fontSize = 20,
                .fontId = FONT_ID_BODY_24,
                .textColor = COLOR_ORANGE,
                .disablePointerEvents = true 
            })); break;
            case 3: CLAY_TEXT(CLAY_STRING("Timeline"), CLAY_TEXT_CONFIG({ 
                .fontSize = 20,
                .fontId = FONT_ID_BODY_24,
                .textColor = COLOR_ORANGE,
                .disablePointerEvents = true 
            })); break;
            case 4: CLAY_TEXT(CLAY_STRING("Routine"), CLAY_TEXT_CONFIG({ 
                .fontSize = 20,
                .fontId = FONT_ID_BODY_24,
                .textColor = COLOR_ORANGE,
                .disablePointerEvents = true 
            })); break;
        }
    }
}

void RenderNavigationMenu() {
    CLAY(CLAY_ID("TopNavigation"), 
        CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIXED(60) },
            .childGap = 16,
            .padding = { 16, 0 },
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }
        }),
        CLAY_RECTANGLE({ .color = COLOR_RED })
    ) {
        RenderNavItem("Home", 0);
        RenderNavItem("Habits", 1);
        RenderNavItem("Todos", 2);
        RenderNavItem("Timeline", 3);
        RenderNavItem("Routine", 4);
    }
}