#define CLAY_EXTEND_CONFIG_RECTANGLE Clay_String link; bool cursorPointer;
#define CLAY_EXTEND_CONFIG_IMAGE Clay_String sourceURL;
#define CLAY_EXTEND_CONFIG_TEXT bool disablePointerEvents;
#define CLAY_IMPLEMENTATION
#include "../vendor/clay/clay.h"

#include "styles.h"
#include "components/nav.h"
#include "pages/home.h"
#include "pages/habits.h"
#include "pages/todos.h"
#include "pages/timeline.h"
#include "pages/routine.h"

double windowWidth = 1024, windowHeight = 768;
uint32_t ACTIVE_PAGE = 1;
uint32_t ACTIVE_RENDERER_INDEX = 0;

// Font IDs
const uint32_t FONT_ID_BODY_16 = 0;
const uint32_t FONT_ID_TITLE_56 = 1;
const uint32_t FONT_ID_BODY_24 = 2;
const uint32_t FONT_ID_BODY_36 = 3;
const uint32_t FONT_ID_TITLE_36 = 4;
const uint32_t FONT_ID_MONOSPACE_24 = 5;

// Colors
// Theme Colors
const Clay_Color COLOR_BACKGROUND = (Clay_Color) {26, 15, 31, 255};  // #1a0f1f
const Clay_Color COLOR_BACKGROUND_HOVER = (Clay_Color) {45, 26, 44, 255};  
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

// Text Colors
const Clay_Color COLOR_TEXT = (Clay_Color) {230, 221, 233, 255};     // #e6dde9

void HandleNavInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        ACTIVE_PAGE = (uint32_t)userData;
    }
}

void RenderCurrentPage() {
    switch(ACTIVE_PAGE) {
        case 0: RenderHomePage(); break;
        case 1: RenderHabitsPage(); break;
        case 2: RenderTodosPage(); break;
        case 3: RenderTimelinePage(); break;
        case 4: RenderRoutinePage(); break;
    }
}

Clay_RenderCommandArray CreateLayout() {
    Clay_BeginLayout();
    CLAY(CLAY_ID("OuterContainer"), 
        CLAY_LAYOUT({ 
            .layoutDirection = CLAY_TOP_TO_BOTTOM, 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() } 
        }), 
        CLAY_RECTANGLE({ .color = COLOR_BACKGROUND })
    ) {
        RenderNavigationMenu();
        
        CLAY(CLAY_ID("MainContent"),
            CLAY_LAYOUT({ 
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
                .padding = { 16, 16 }
            })
        ) {
            RenderCurrentPage();
        }
    }
    return Clay_EndLayout();
}

CLAY_WASM_EXPORT("UpdateDrawFrame") Clay_RenderCommandArray UpdateDrawFrame(
    float width, 
    float height, 
    float mouseWheelX, 
    float mouseWheelY, 
    float mousePositionX, 
    float mousePositionY, 
    bool isTouchDown, 
    bool isMouseDown, 
    bool arrowKeyDownPressedThisFrame, 
    bool arrowKeyUpPressedThisFrame, 
    bool dKeyPressedThisFrame, 
    float deltaTime
) {
    windowWidth = width;
    windowHeight = height;
    Clay_SetLayoutDimensions((Clay_Dimensions) { width, height });
    Clay_SetPointerState((Clay_Vector2) {mousePositionX, mousePositionY}, isMouseDown || isTouchDown);
    return CreateLayout();
}


int main() {
    return 0;
}
