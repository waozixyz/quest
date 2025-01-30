// config.c
#include "config.h"
// Configuration variables
double windowWidth = 400, windowHeight = 768;
float globalScalingFactor = 1.0f;
uint32_t ACTIVE_PAGE = 0;
uint32_t ACTIVE_RENDERER_INDEX = 0;

// Breakpoints
const float BREAKPOINT_LARGE = 1024.0f;
const float BREAKPOINT_MEDIUM = 640.0f;
const float BREAKPOINT_SMALL = 480.0f;
const float BREAKPOINT_XSMALL = 420.0f;

const int DEFAULT_PADDING = 32;
void HandleNavInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        uint32_t new_page = (uint32_t)userData;
        if (new_page != ACTIVE_PAGE) {
            printf("Page changing from %u to %u\n", ACTIVE_PAGE, new_page);

            // Reset Clay's internal caches
            Clay_ResetMeasureTextCache();
            
            // Force Clay to update layout state on next frame
            Clay_SetLayoutDimensions((Clay_Dimensions){windowWidth, windowHeight});

            ACTIVE_PAGE = new_page;
        }
    }
}