// config.h
#ifndef CONFIG_H
#define CONFIG_H

#include "clay_extensions.h"
#include "clay.h"
#include <stdio.h>
// Window & UI configuration
extern double windowWidth, windowHeight;
extern float globalScalingFactor;
extern uint32_t ACTIVE_PAGE;
extern uint32_t ACTIVE_RENDERER_INDEX;
    
// Breakpoints
extern const float BREAKPOINT_LARGE;
extern const float BREAKPOINT_MEDIUM;
extern const float BREAKPOINT_SMALL;

// Font IDs
extern const uint32_t FONT_ID_BODY_16;
extern const uint32_t FONT_ID_TITLE_56;
extern const uint32_t FONT_ID_BODY_24;
extern const uint32_t FONT_ID_BODY_36;
extern const uint32_t FONT_ID_TITLE_36;
extern const uint32_t FONT_ID_MONOSPACE_24;
extern const uint32_t FONT_ID_BODY_14;
extern const uint32_t FONT_ID_BODY_18;

// Colors
extern const Clay_Color COLOR_BACKGROUND;
extern const Clay_Color COLOR_BACKGROUND_HOVER;
extern const Clay_Color COLOR_BACKGROUND_FOCUSED;
extern const Clay_Color COLOR_PRIMARY;
extern const Clay_Color COLOR_PRIMARY_HOVER;
extern const Clay_Color COLOR_SECONDARY;
extern const Clay_Color COLOR_ACCENT;
extern const Clay_Color COLOR_CARD;
extern const Clay_Color COLOR_CARD_HOVER;
extern const Clay_Color COLOR_PANEL;
extern const Clay_Color COLOR_SUCCESS;
extern const Clay_Color COLOR_WARNING;
extern const Clay_Color COLOR_ERROR;
extern const Clay_Color COLOR_ERROR_HOVER;
extern const Clay_Color COLOR_INFO;
extern const Clay_Color COLOR_BORDER;
extern const Clay_Color COLOR_BORDER_FOCUSED;
extern const Clay_Color COLOR_TEXT;
extern const Clay_Color COLOR_TEXT_SECONDARY;
extern const Clay_Color COLOR_CURSOR;
extern const Clay_Color COLOR_DANGER;
extern const int DEFAULT_PADDING;


extern const Clay_Color COLOR_NAV_BACKGROUND;
extern const Clay_Color COLOR_NAV_ITEM_BACKGROUND;
extern const Clay_Color COLOR_NAV_ITEM_BACKGROUND_ACTIVE;
extern const Clay_Color COLOR_NAV_ITEM_TEXT;
extern const Clay_Color COLOR_NAV_ITEM_TEXT_ACTIVE;
extern const Clay_Color COLOR_NAV_SHADOW;

// Navigation handler
void HandleNavInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData);



typedef struct {
    float delta_time;
    bool isTextInput;
    char text[32];
    int key;
} InputEvent;

#endif