#include "components/card.h"

void HandleMinimizeClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        bool* is_minimized = (bool*)userData;
        *is_minimized = !*is_minimized;
    }
}

void RenderCard(const char* title, int card_id, bool* is_minimized, void (*render_content)()) {
    static const Clay_String MINIMIZE_PLUS = { .length = 1, .chars = "+" };
    static const Clay_String MINIMIZE_MINUS = { .length = 1, .chars = "-" };

    CLAY(CLAY_IDI("Card", card_id),
        CLAY_LAYOUT({
            .sizing = { 
                windowWidth < BREAKPOINT_SMALL ? CLAY_SIZING_GROW() : CLAY_SIZING_FIXED(800), // Increased from 400 to 800
                CLAY_SIZING_FIT() 
            },
            .childGap = 16, // Increased from 8 to 16
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        }),
        CLAY_RECTANGLE({
            .color = COLOR_PANEL,
            .cornerRadius = CLAY_CORNER_RADIUS(12) // Increased from 8 to 12
        })
    ) {
        // Card Header
        CLAY(CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIXED(60) }, // Increased from 40 to 60
            .padding = { 16, 16, 16, 16 }, // Increased from 8 to 16
            .childGap = 16, // Increased from 8 to 16
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }
        }),
        CLAY_RECTANGLE({
            .color = COLOR_SECONDARY,
            .cornerRadius = CLAY_CORNER_RADIUS(8) // Increased from 4 to 8
        })) {
            // Title
            CLAY(CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT() }
            })) {
                Clay_String title_str = {
                    .length = strlen(title),
                    .chars = title
                };
                CLAY_TEXT(title_str, 
                    CLAY_TEXT_CONFIG({
                        .fontSize = 24, // Increased from 16 to 24
                        .fontId = FONT_ID_BODY_24, // Changed from BODY_16 to BODY_24
                        .textColor = COLOR_TEXT
                    })
                );
            }
            
            // Minimize button
            CLAY(CLAY_IDI("MinimizeButton", card_id),
                CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_FIXED(40), CLAY_SIZING_FIXED(40) }, // Increased from 24 to 40
                    .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
                }),
                CLAY_RECTANGLE({
                    .color = Clay_Hovered() ? COLOR_PRIMARY_HOVER : COLOR_PANEL,
                    .cornerRadius = CLAY_CORNER_RADIUS(8), // Increased from 4 to 8
                }),
                Clay_OnHover(HandleMinimizeClick, (intptr_t)is_minimized)
            ) {
                CLAY(CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_FIT(), CLAY_SIZING_FIT() },
                    .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
                })) {
                    CLAY_TEXT(
                        *is_minimized ? MINIMIZE_PLUS : MINIMIZE_MINUS,
                        CLAY_TEXT_CONFIG({
                            .fontSize = 24, // Increased from 16 to 24
                            .fontId = FONT_ID_BODY_24, // Changed from BODY_16 to BODY_24
                            .textColor = COLOR_TEXT
                        })
                    );
                }
            }
        }

        // Card Content
        if (!*is_minimized) {
            CLAY(CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT() },
                .padding = { 24, 24, 24, 24 } // Increased from 16 to 24
            })) {
                render_content();
            }
        }
    }
}