#include "pages/home.h"
#include "components/home_card.h"

void SetupCardGridLayout() {
    CLAY_LAYOUT({
        .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
        .childGap = DEFAULT_PADDING,
        .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
    });
}


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
        // Cards container
        CLAY(CLAY_ID("CardGrid"), SetupCardGridLayout()) {
            if (screenWidth < BREAKPOINT_SMALL) {
                // Mobile layout - 1 card per row
                CLAY(CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
                    .childGap = 32,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM
                })) {
                    for(uint32_t i = 0; i < 4; i++) {
                        RenderHomeCard(i);
                    }
                }
            } else if (screenWidth < BREAKPOINT_LARGE) {
                // Small screen layout - 2x2 grid
                CLAY(CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
                    .childGap = 32,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM
                })) {
                    for (int row = 0; row < 2; ++row) {
                        CLAY(SetupCardGridLayout()) {
                            RenderHomeCard(row * 2);
                            RenderHomeCard(row * 2 + 1);
                        }
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
