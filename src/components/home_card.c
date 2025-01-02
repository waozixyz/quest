#include "home_card.h"

void RenderHomeCard(uint32_t index) {
    CLAY(CLAY_IDI("HomeCard", index), 
        CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_PERCENT(0.25f), CLAY_SIZING_FIXED(200) },
            .padding = { 24, 24 },
            .childGap = 16,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        }),
        CLAY_RECTANGLE({ 
            .color = Clay_Hovered() ? COLOR_LIGHT_HOVER : COLOR_LIGHT,
            .cornerRadius = CLAY_CORNER_RADIUS(10),
            .cursorPointer = true
        }),
        CLAY_BORDER_OUTSIDE_RADIUS(2, COLOR_RED, 10),
        Clay_OnHover(HandleNavInteraction, index + 1)
    ) {
        switch(index) {
            case 0: 
                CLAY_TEXT(CLAY_STRING("Habits"), 
                    CLAY_TEXT_CONFIG({ 
                        .fontSize = 28,
                        .fontId = FONT_ID_TITLE_36,
                        .textColor = COLOR_ORANGE
                    })
                );
                
                CLAY_TEXT(CLAY_STRING("Track and maintain your daily habits for better living"), 
                    CLAY_TEXT_CONFIG({ 
                        .fontSize = 16,
                        .fontId = FONT_ID_BODY_16,
                        .textColor = COLOR_ORANGE
                    })
                );
                break;
            
            case 1:
                CLAY_TEXT(CLAY_STRING("Todos"), 
                    CLAY_TEXT_CONFIG({ 
                        .fontSize = 28,
                        .fontId = FONT_ID_TITLE_36,
                        .textColor = COLOR_ORANGE
                    })
                );
                
                CLAY_TEXT(CLAY_STRING("Manage your tasks and stay organized"), 
                    CLAY_TEXT_CONFIG({ 
                        .fontSize = 16,
                        .fontId = FONT_ID_BODY_16,
                        .textColor = COLOR_ORANGE
                    })
                );
                break;

            case 2:
                CLAY_TEXT(CLAY_STRING("Timeline"), 
                    CLAY_TEXT_CONFIG({ 
                        .fontSize = 28,
                        .fontId = FONT_ID_TITLE_36,
                        .textColor = COLOR_ORANGE
                    })
                );
                
                CLAY_TEXT(CLAY_STRING("View your life's journey and important moments"), 
                    CLAY_TEXT_CONFIG({ 
                        .fontSize = 16,
                        .fontId = FONT_ID_BODY_16,
                        .textColor = COLOR_ORANGE
                    })
                );
                break;

            case 3:
                CLAY_TEXT(CLAY_STRING("Routine"), 
                    CLAY_TEXT_CONFIG({ 
                        .fontSize = 28,
                        .fontId = FONT_ID_TITLE_36,
                        .textColor = COLOR_ORANGE
                    })
                );
                
                CLAY_TEXT(CLAY_STRING("Plan and track your daily routines"), 
                    CLAY_TEXT_CONFIG({ 
                        .fontSize = 16,
                        .fontId = FONT_ID_BODY_16,
                        .textColor = COLOR_ORANGE
                    })
                );
                break;
        }
    }
}