#include <stdio.h>
#include "../../vendor/clay/clay.h" 
#include "color_picker.h"  

ColorPickerState habit_color_picker = {0};


void HandleColorSelection(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        size_t color_index = (size_t)userData;
        
        // Directly use the color index to update the selected color
        if (color_index < COLOR_PALETTE_SIZE) {
            habit_color_picker.selected_color = COLOR_PALETTE[color_index];
            
            // Trigger color change callback if set
            if (habit_color_picker.on_color_change) {
                habit_color_picker.on_color_change(
                    habit_color_picker.selected_color,
                    habit_color_picker.user_data
                );
            }
        }
    }
}

void RenderColorPicker(ColorPickerState* state) {
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
                .color = state->selected_color,
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
                    // Pass combined state and index
                    Clay_OnHover(HandleColorSelection, (intptr_t)i)

                );
            }
        }
    }
}