#include "components/color_picker.h"  

#include "rocks.h"
#include "quest_theme.h"

// Static variables1
static void (*g_color_change_callback)(Clay_Color) = NULL;
static Modal* g_modal = NULL;

// Event handlers
static void HandleColorHover(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        size_t color_index = (size_t)userData;
        if (color_index < COLOR_PALETTE_SIZE && g_color_change_callback) {
            g_color_change_callback(COLOR_PALETTE[color_index]);
            g_modal->is_open = false;
        }
    }
}

static void HandleCurrentColorClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        Modal* modal = (Modal*)userData;
        OpenModal(modal);
    }
}

// Helper function to render a single color option
static void RenderColorOption(size_t index) {
    RocksTheme base_theme = rocks_get_theme(g_rocks);
    QuestThemeExtension* theme = (QuestThemeExtension*)base_theme.extension;

    CLAY(CLAY_IDI("ModalColorOption", index),
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_FIXED(40), CLAY_SIZING_FIXED(40) }
        }),
        CLAY_RECTANGLE({
            .color = COLOR_PALETTE[index],
            .cornerRadius = CLAY_CORNER_RADIUS(4)
        }),
        Clay_OnHover(HandleColorHover, (intptr_t)index)
    );
}

// Helper function to render a row of colors
static void RenderColorRow(size_t start_index, size_t end_index) {
    CLAY(CLAY_IDI("ColorGridRow", start_index),
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT() },
            .childGap = 10,
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
        })
    ) {
        for (size_t i = start_index; i < end_index; i++) {
            RenderColorOption(i);
        }
    }
}

static void RenderColorPaletteModal(void) {
    RocksTheme base_theme = rocks_get_theme(g_rocks);
    QuestThemeExtension* theme = (QuestThemeExtension*)base_theme.extension;

    CLAY(CLAY_ID("ModalColorPalette"),
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT() },
            .childGap = 10,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        })
    ) {
        CLAY_TEXT(CLAY_STRING("Select a Color"), 
            CLAY_TEXT_CONFIG({
                .fontSize = 18,
                .fontId = FONT_ID_BODY_24,
                .textColor = base_theme.text
            })
        );

        CLAY(CLAY_ID("ColorGrid"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT() },
                .childGap = 10,
                .layoutDirection = CLAY_TOP_TO_BOTTOM
            })
        ) {
            // First row
            RenderColorRow(0, COLORS_PER_ROW);
            
            // Second row
            RenderColorRow(COLORS_PER_ROW, COLORS_PER_ROW * 2);
            
            // Third row
            RenderColorRow(COLORS_PER_ROW * 2, COLOR_PALETTE_SIZE);
        }
    }
}

void RenderColorPicker(Clay_Color current_color, void (*on_color_change)(Clay_Color), Modal* modal) {
    RocksTheme base_theme = rocks_get_theme(g_rocks);
    QuestThemeExtension* theme = (QuestThemeExtension*)base_theme.extension;

    g_color_change_callback = on_color_change;
    g_modal = modal;

    CLAY(CLAY_ID("ColorPickerContainer"),
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT() },
            .childGap = 10,
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
            .padding = { 16, 16, 16, 16 }
        })
    ) {
        // Current Color Display with click handler
        CLAY(CLAY_ID("CurrentColorDisplay"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_FIXED(40), CLAY_SIZING_FIXED(40) }
            }),
            CLAY_RECTANGLE({
                .color = current_color,
                .cornerRadius = CLAY_CORNER_RADIUS(8),
                .cursorPointer = true
            }),
            Clay_OnHover(HandleCurrentColorClick, (intptr_t)modal)
        );
    }

    // Render the modal if it's open
    RenderModal(modal, RenderColorPaletteModal);
}