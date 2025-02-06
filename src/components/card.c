#include "components/card.h"

#include "rocks.h"
#include "quest_theme.h"

void HandleMinimizeClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        bool* is_minimized = (bool*)userData;
        *is_minimized = !*is_minimized;
    }
}

void RenderCard(const char* title, int card_id, bool* is_minimized, void (*render_content)()) {
    Rocks_Theme base_theme = Rocks_GetTheme(GRocks);
    QuestThemeExtension* theme = (QuestThemeExtension*)base_theme.extension;

    static const Clay_String MINIMIZE_PLUS = { .length = 1, .chars = "+" };
    static const Clay_String MINIMIZE_MINUS = { .length = 1, .chars = "-" };

    CLAY(CLAY_IDI("Card", card_id),
        CLAY_LAYOUT({
            .sizing = { 
                CLAY_SIZING_GROW(),
                CLAY_SIZING_FIT() 
            },
            .childGap = 16,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        }),
        CLAY_RECTANGLE({
            .color = theme->card,
            .cornerRadius = CLAY_CORNER_RADIUS(12)
        })
    ) {
        // Card Header
        CLAY(CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIXED(60) },
            .padding = { 16, 16, 16, 16 },
            .childGap = 16,
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }
        }),
        CLAY_RECTANGLE({
            .color = base_theme.secondary,
            .cornerRadius = CLAY_CORNER_RADIUS(8)
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
                        .fontSize = 24,
                        .fontId = FONT_ID_BODY_24,
                        .textColor = base_theme.text
                    })
                );
            }
            
            // Minimize button
            CLAY(CLAY_IDI("MinimizeButton", card_id),
                CLAY_LAYOUT({
                    .sizing = { CLAY_SIZING_FIXED(40), CLAY_SIZING_FIXED(40) },
                    .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
                }),
                CLAY_RECTANGLE({
                    .color = Clay_Hovered() ? base_theme.primary_hover : theme->card,
                    .cornerRadius = CLAY_CORNER_RADIUS(8),
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
                            .fontSize = 24,
                            .fontId = FONT_ID_BODY_24,
                            .textColor = base_theme.text
                        })
                    );
                }
            }
        }

        // Card Content
        if (!*is_minimized) {
            CLAY(CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT() },
                .padding = { 24, 24, 24, 24 }
            })) {
                render_content();
            }
        }
    }
}