#include "calendar_box.h"
#include <stdio.h>

static const Clay_String DAY_STRINGS[32] = {
    {1, "0"}, {1, "1"}, {1, "2"}, {1, "3"}, {1, "4"}, 
    {1, "5"}, {1, "6"}, {1, "7"}, {1, "8"}, {1, "9"},
    {2, "10"}, {2, "11"}, {2, "12"}, {2, "13"}, {2, "14"},
    {2, "15"}, {2, "16"}, {2, "17"}, {2, "18"}, {2, "19"},
    {2, "20"}, {2, "21"}, {2, "22"}, {2, "23"}, {2, "24"},
    {2, "25"}, {2, "26"}, {2, "27"}, {2, "28"}, {2, "29"},
    {2, "30"}, {2, "31"}
};
void RenderCalendarBox(CalendarBoxProps props) {    
    Clay_Color box_color = COLOR_SECONDARY;
    Clay_Color hover_color = COLOR_BOX_HOVER;

    if (props.is_past) {
        box_color.r *= 0.7f;
        box_color.g *= 0.7f;
        box_color.b *= 0.7f;
        hover_color.r *= 0.7f;
        hover_color.g *= 0.7f;
        hover_color.b *= 0.7f;
    }

    if (props.is_completed) {
        box_color = COLOR_BOX_COMPLETED_ACTIVE;
        hover_color = COLOR_BOX_COMPLETED_HOVER;
    }

    CLAY(CLAY_IDI("Box", props.unique_index),
        CLAY_LAYOUT({
            .sizing = { 
                CLAY_SIZING_FIXED(70),
                CLAY_SIZING_FIXED(70)  
            },
            .childAlignment = { 
                .x = CLAY_ALIGN_X_CENTER,
                .y = CLAY_ALIGN_Y_CENTER 
            }
        }),
        CLAY_RECTANGLE({ 
            .color = Clay_Hovered() ? hover_color : box_color,
            .cornerRadius = CLAY_CORNER_RADIUS(8)
        }),
        props.is_today ? CLAY_BORDER_OUTSIDE_RADIUS(2, COLOR_PRIMARY, 8) : 0,
        Clay_OnHover(props.on_click, props.unique_index)  // Changed from Clay_OnClick to Clay_OnHover
    ) {
        CLAY_TEXT(DAY_STRINGS[props.day_number], 
            CLAY_TEXT_CONFIG({ 
                .fontSize = 16,
                .fontId = FONT_ID_BODY_16,
                .textColor = COLOR_TEXT
            })
        );
    }
}
