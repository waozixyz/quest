#include "home_card.h"


const int CONER_RADIUS = 10;

void RenderHomeCard(uint32_t index) {
    float screenWidth = (float)windowWidth;

    float cardWidthPercent = screenWidth < 768 ? 0.5f : 0.25f;


    Clay_TextElementConfig card_title_config = *CLAY_TEXT_CONFIG({ 
        .fontSize = 28,
        .fontId = FONT_ID_TITLE_36,
        .textColor = COLOR_TEXT,
        .disablePointerEvents = true  // Add this line
    });

    Clay_TextElementConfig card_body_config = *CLAY_TEXT_CONFIG({ 
        .fontSize = 16,
        .fontId = FONT_ID_BODY_16,
        .textColor = COLOR_TEXT,
        .disablePointerEvents = true  // Add this line
    });

    CLAY(CLAY_IDI("HomeCard", index), 
        CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_PERCENT(cardWidthPercent), CLAY_SIZING_FIXED(200) },
            .padding = { 24, 24 },
            .childGap = 16,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        }),
        CLAY_RECTANGLE({ 
            .color = Clay_Hovered() ? COLOR_CARD_HOVER : COLOR_CARD,
            .cornerRadius = CLAY_CORNER_RADIUS(CONER_RADIUS),
            .cursorPointer = true

        }),
        CLAY_BORDER({
            .top = Clay_Hovered() ? (Clay_Border){ .width = 2, .color = COLOR_PRIMARY } : (Clay_Border){ .width = 2, .color = COLOR_BACKGROUND },
            .cornerRadius = { 
                .topLeft = CONER_RADIUS,
                .topRight = CONER_RADIUS, 
                .bottomLeft = 0, 
                .bottomRight = 0 
            }
        }),
        
        Clay_OnHover(HandleNavInteraction, index + 1)
    ) {
        
        switch(index) {
            case 0: 
                CLAY_TEXT(CLAY_STRING("Habits"), &card_title_config);
                
                CLAY_TEXT(CLAY_STRING("Track and maintain your daily habits for better living"), &card_body_config);
                break;
            
            case 1:
                CLAY_TEXT(CLAY_STRING("Todos"), &card_title_config);
                
                CLAY_TEXT(CLAY_STRING("Manage your tasks and stay organized"), &card_body_config);
                break;

            case 2:
                CLAY_TEXT(CLAY_STRING("Timeline"), &card_title_config);
                
                CLAY_TEXT(CLAY_STRING("View your life's journey and important moments"), &card_body_config);
                break;

            case 3:
                CLAY_TEXT(CLAY_STRING("Routine"), &card_title_config);
                
                CLAY_TEXT(CLAY_STRING("Plan and track your daily routines"), &card_body_config);
                break;
        }
    }
}