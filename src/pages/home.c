#include "home.h"
#include "../components/home_card.h"
#include "../styles.h"

void RenderHomePage() {
    float screenWidth = (float)windowWidth;
    CLAY(CLAY_ID("HomeContainer"), 
        CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .padding = { 32, 32 },
            .childGap = 32,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        })
    ) {
        // Title
        CLAY(CLAY_ID("HomePageTitle"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) },
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
                .padding = { 0, 32 }
            })
        ) {
            CLAY_TEXT(CLAY_STRING("Welcome to MyQuest"),
                CLAY_TEXT_CONFIG({
                    .fontSize = 48,
                    .fontId = FONT_ID_TITLE_56,
                    .textColor = COLOR_TEXT
                })
            );
        }

        // Cards container
        CLAY(CLAY_ID("CardGrid"), 
            CLAY_LAYOUT({ 
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
                .childGap = 32,
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
            })
        ) {
            if (screenWidth < 768) {
                // Small screen layout - 2x2 grid
                CLAY(CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
                    .childGap = 32,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM
                })) {
                    // First row
                    CLAY(CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
                        .childGap = 32,
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
                    })) {
                        RenderHomeCard(0);
                        RenderHomeCard(1);
                    }
                    
                    // Second row
                    CLAY(CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
                        .childGap = 32,
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
                    })) {
                        RenderHomeCard(2);
                        RenderHomeCard(3);
                    }
                }
            } else {
                // Large screen layout - single row
                for(uint32_t i = 0; i < 4; i++) {
                    RenderHomeCard(i);
                }
            }
        }
    }
}