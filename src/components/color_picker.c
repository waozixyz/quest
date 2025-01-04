#include <stdio.h>
#include "../../vendor/clay/clay.h" 
#include "color_picker.h"  

static void (*g_color_change_callback)(Clay_Color) = NULL;


static void HandleColorHover(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        size_t color_index = (size_t)userData;
        if (color_index < COLOR_PALETTE_SIZE && g_color_change_callback) {
            g_color_change_callback(COLOR_PALETTE[color_index]);
        }
    }
}


void RenderColorPicker(Clay_Color current_color, void (*on_color_change)(Clay_Color)) {
    // Store the callback globally so the wrapper can access it
    g_color_change_callback = on_color_change;

    CLAY(CLAY_ID("ColorPickerContainer"),
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) },
            .childGap = 10,
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
            .padding = { 16, 16 }
        })
    ) {
        // Current Color Display
        CLAY(CLAY_ID("CurrentColorDisplay"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_FIXED(40), CLAY_SIZING_FIXED(40) }
            }),
            CLAY_RECTANGLE({
                .color = current_color,
                .cornerRadius = CLAY_CORNER_RADIUS(8)
            })
        );

        // Color Palette
        CLAY(CLAY_ID("ColorPaletteContainer"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) },
                .childGap = 10,
                .layoutDirection = CLAY_LEFT_TO_RIGHT
            })
        ) {
            for (size_t i = 0; i < COLOR_PALETTE_SIZE; i++) {                
                CLAY(CLAY_IDI("ColorOption", i),
                    CLAY_LAYOUT({
                        .sizing = { CLAY_SIZING_FIXED(30), CLAY_SIZING_FIXED(30) }
                    }),
                    CLAY_RECTANGLE({
                        .color = COLOR_PALETTE[i],
                        .cornerRadius = CLAY_CORNER_RADIUS(4)
                    }),
                    Clay_OnHover(HandleColorHover, (intptr_t)i)

                );
            }
        }
    }
}