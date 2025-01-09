// config.c
#include "config.h"

// Configuration variables
double windowWidth = 350, windowHeight = 768;
float globalScalingFactor = 1.0f;
uint32_t ACTIVE_PAGE = 1;
uint32_t ACTIVE_RENDERER_INDEX = 0;

// Breakpoints
const float BREAKPOINT_LARGE = 1024.0f;
const float BREAKPOINT_MEDIUM = 640.0f;
const float BREAKPOINT_SMALL = 480.0f;

// Font IDs
const uint32_t FONT_ID_BODY_16 = 0;
const uint32_t FONT_ID_TITLE_56 = 1;
const uint32_t FONT_ID_BODY_24 = 2;
const uint32_t FONT_ID_BODY_36 = 3;
const uint32_t FONT_ID_TITLE_36 = 4;
const uint32_t FONT_ID_MONOSPACE_24 = 5;

// Theme Colors
const Clay_Color COLOR_BACKGROUND = (Clay_Color) {26, 15, 31, 255};  // #1a0f1f
const Clay_Color COLOR_BACKGROUND_HOVER = (Clay_Color) {45, 26, 44, 255};  
const Clay_Color COLOR_BACKGROUND_FOCUSED = (Clay_Color) {35, 20, 40, 255}; // #231428
const Clay_Color COLOR_PRIMARY = (Clay_Color) {148, 47, 74, 255};    // #942f4a
const Clay_Color COLOR_PRIMARY_HOVER = (Clay_Color) {177, 54, 88, 255}; // #b13658
const Clay_Color COLOR_SECONDARY = (Clay_Color) {74, 38, 57, 255};   // #4a2639
const Clay_Color COLOR_ACCENT = (Clay_Color) {255, 107, 151, 255};   // #ff6b97

// UI Element Colors
const Clay_Color COLOR_CARD = (Clay_Color) {45, 31, 51, 255};       // #2d1f33
const Clay_Color COLOR_CARD_HOVER = (Clay_Color) {61, 42, 66, 255};  // #3d2a42
const Clay_Color COLOR_PANEL = (Clay_Color) {45, 31, 51, 242};      // #2d1f33f2

// State Colors
const Clay_Color COLOR_SUCCESS = (Clay_Color) {76, 175, 80, 255};    // #4caf50
const Clay_Color COLOR_WARNING = (Clay_Color) {255, 152, 0, 255};    // #ff9800
const Clay_Color COLOR_ERROR = (Clay_Color) {244, 67, 54, 255};      // #f44336
const Clay_Color COLOR_ERROR_HOVER = (Clay_Color) {255, 87, 74, 255}; // #ff574a
const Clay_Color COLOR_INFO = (Clay_Color) {33, 150, 243, 255};      // #2196f3

// Border Colors
const Clay_Color COLOR_BORDER = (Clay_Color) {74, 38, 57, 255};   // #4a2639
const Clay_Color COLOR_BORDER_FOCUSED = (Clay_Color) {148, 47, 74, 255}; // #942f4a

// Text Colors
const Clay_Color COLOR_TEXT = (Clay_Color) {230, 221, 233, 255};     // #e6dde9
const Clay_Color COLOR_TEXT_SECONDARY = (Clay_Color) {184, 168, 192, 255}; // #b8a8c0
const Clay_Color COLOR_CURSOR = (Clay_Color){255, 107, 151, 255};    // Using COLOR_ACCENT for cursor

const int DEFAULT_PADDING = 32;

// Navigation handler implementation
void HandleNavInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        ACTIVE_PAGE = (uint32_t)userData;
    }
}

#ifndef __EMSCRIPTEN__
// Scroll state variables
bool isScrollDragging = false;
int scrollDragStartY = 0;
int scrollDragStartX = 0;
Clay_Vector2 initialScrollPosition = {0, 0};
bool isScrollThumbDragging = false;
bool isHorizontalScrollThumbDragging = false;
bool hadMotionBetweenDownAndUp = false;
Clay_Vector2 initialPointerPosition = {0, 0};
#endif