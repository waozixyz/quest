#include "components/calendar_box.h"
#include "config.h"
static const Clay_String DAY_STRINGS[32] = {
    {1, "0"}, {1, "1"}, {1, "2"}, {1, "3"}, {1, "4"}, 
    {1, "5"}, {1, "6"}, {1, "7"}, {1, "8"}, {1, "9"},
    {2, "10"}, {2, "11"}, {2, "12"}, {2, "13"}, {2, "14"},
    {2, "15"}, {2, "16"}, {2, "17"}, {2, "18"}, {2, "19"},
    {2, "20"}, {2, "21"}, {2, "22"}, {2, "23"}, {2, "24"},
    {2, "25"}, {2, "26"}, {2, "27"}, {2, "28"}, {2, "29"},
    {2, "30"}, {2, "31"}
};
static Clay_Color ClampColor(Clay_Color color) {
    return (Clay_Color){
        .r = fminf(fmaxf(color.r / 255.0f, 0.0f), 1.0f) * 255.0f,
        .g = fminf(fmaxf(color.g / 255.0f, 0.0f), 1.0f) * 255.0f,
        .b = fminf(fmaxf(color.b / 255.0f, 0.0f), 1.0f) * 255.0f,
        .a = fminf(fmaxf(color.a / 255.0f, 0.0f), 1.0f) * 255.0f
    };
}

static Clay_Color GenerateHoverColor(Clay_Color color) {
    return ClampColor((Clay_Color){
        .r = fminf(color.r * 1.3f, 255.0f),
        .g = fminf(color.g * 1.3f, 255.0f),
        .b = fminf(color.b * 1.3f, 255.0f),
        .a = color.a
    });
}

static Clay_Color GeneratePastColor(Clay_Color color) {
    return ClampColor((Clay_Color){
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a
    });
}

static Clay_Color GenerateCompletedColor(Clay_Color color) {
    return ClampColor((Clay_Color){
        .r = color.r * 1.1f,
        .g = color.g * 1.1f,
        .b = color.b * 1.1f,
        .a = 200
    });
}

static Clay_Color GenerateFutureColor(Clay_Color color) {
    return ClampColor((Clay_Color){
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a
    });
}
void RenderCalendarBox(CalendarBoxProps props) {    
    float screenWidth = (float)windowWidth;
    const float MAX_BOX_SIZE = 90.0f;  // Maximum box size
    const float MIN_BOX_SIZE = 32.0f;  // Minimum box size

    // Calculate box size as a percentage of screen width
    float boxSize = screenWidth * 0.1f;  // 10% of screen width
    
    // Clamp the box size between min and max
    boxSize = fmaxf(MIN_BOX_SIZE, fminf(boxSize, MAX_BOX_SIZE));

    // Rest of the existing color generation logic remains the same
    Clay_Color base_color = props.custom_color;
    base_color.a = 70.0f;

    Clay_Color box_color = base_color;
    Clay_Color hover_color = base_color;
    
    if (props.is_past) {
        box_color = GeneratePastColor(base_color);
        hover_color = GenerateHoverColor(box_color);
    } else if (!props.is_past && !props.is_today) {
        // Future days
        box_color = GenerateFutureColor(base_color);
        hover_color = GenerateHoverColor(base_color);
    }

    // Completed state handling
    if (props.is_completed) {
        box_color = GenerateCompletedColor(base_color);
        hover_color = GenerateHoverColor(box_color);
    }

    // Today's border color - more vibrant and distinct
    Clay_Color today_border_color = ClampColor((Clay_Color){
        .r = base_color.r * 1.5f,
        .g = base_color.g * 1.5f,
        .b = base_color.b * 1.5f,
        .a = 255.0f  // Full opacity for today's border
    });

    // Determine font size based on box size
    int fontSize = (int)(boxSize * 0.25f);

    CLAY(CLAY_IDI("Box", props.unique_index),
        CLAY_LAYOUT({
            .sizing = { 
                CLAY_SIZING_FIXED(boxSize),
                CLAY_SIZING_FIXED(boxSize)  
            },
            .childAlignment = { 
                .x = CLAY_ALIGN_X_CENTER,
                .y = CLAY_ALIGN_Y_CENTER 
            }
        }),
        CLAY_RECTANGLE({ 
            .color = Clay_Hovered() ? hover_color : box_color,
            .cornerRadius = CLAY_CORNER_RADIUS(8),
            .cursorPointer = true
        }),
        // Distinct border for today
        props.is_today ? CLAY_BORDER_OUTSIDE_RADIUS(2, today_border_color, 8) : 0,
        (props.is_past || props.is_today) ? Clay_OnHover(props.on_click, props.unique_index) : 0
    ) {
        CLAY_TEXT(DAY_STRINGS[props.day_number], 
            CLAY_TEXT_CONFIG({ 
                .fontSize = fontSize,
                .fontId = FONT_ID_BODY_16,
                .textColor = COLOR_TEXT
            })
        );
    }
}