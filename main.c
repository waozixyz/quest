#define CLAY_EXTEND_CONFIG_RECTANGLE Clay_String link; bool cursorPointer;
#define CLAY_EXTEND_CONFIG_IMAGE Clay_String sourceURL;
#define CLAY_EXTEND_CONFIG_TEXT bool disablePointerEvents;
#define CLAY_IMPLEMENTATION
#include "./vendor/clay/clay.h"

double windowWidth = 1024, windowHeight = 768;
uint32_t ACTIVE_PAGE = 0; // 0 = home, 1 = habits, 2 = todos, 3 = timeline, 4 = routine
uint32_t ACTIVE_RENDERER_INDEX = 0;


const uint32_t FONT_ID_BODY_16 = 0;
const uint32_t FONT_ID_TITLE_56 = 1;
const uint32_t FONT_ID_BODY_24 = 2;
const uint32_t FONT_ID_BODY_36 = 3;
const uint32_t FONT_ID_TITLE_36 = 4;
const uint32_t FONT_ID_MONOSPACE_24 = 5;
// Base colors
const Clay_Color COLOR_LIGHT = (Clay_Color) {40, 35, 40, 255};  // Slightly lighter dark background
const Clay_Color COLOR_LIGHT_HOVER = (Clay_Color) {55, 45, 55, 255}; // Hover state
const Clay_Color COLOR_RED = (Clay_Color) {220, 53, 69, 255}; // Main crimson accent
const Clay_Color COLOR_RED_HOVER = (Clay_Color) {240, 73, 89, 255}; // Brighter crimson for hover
const Clay_Color COLOR_ORANGE = (Clay_Color) {255, 245, 245, 255}; // Light text color
const Clay_Color COLOR_BLUE = (Clay_Color) {150, 60, 80, 255}; // Secondary accent

// Gradient colors for visual interest
const Clay_Color COLOR_TOP_BORDER_1 = (Clay_Color) {220, 53, 69, 255};  // Crimson
const Clay_Color COLOR_TOP_BORDER_2 = (Clay_Color) {230, 63, 79, 255};
const Clay_Color COLOR_TOP_BORDER_3 = (Clay_Color) {240, 73, 89, 255};
const Clay_Color COLOR_TOP_BORDER_4 = (Clay_Color) {250, 83, 99, 255};
const Clay_Color COLOR_TOP_BORDER_5 = (Clay_Color) {255, 93, 109, 255};

// Blob borders with similar gradient
const Clay_Color COLOR_BLOB_BORDER_1 = (Clay_Color) {220, 53, 69, 255};
const Clay_Color COLOR_BLOB_BORDER_2 = (Clay_Color) {230, 63, 79, 255};
const Clay_Color COLOR_BLOB_BORDER_3 = (Clay_Color) {240, 73, 89, 255};
const Clay_Color COLOR_BLOB_BORDER_4 = (Clay_Color) {250, 83, 99, 255};
const Clay_Color COLOR_BLOB_BORDER_5 = (Clay_Color) {255, 93, 109, 255};

void HandleNavInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        ACTIVE_PAGE = (uint32_t)userData;
    }
}
void RenderNavigationMenu() {
    CLAY(CLAY_ID("TopNavigation"), 
        CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIXED(60) },
            .childGap = 16,
            .padding = { 16, 0 },
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }
        }),
        CLAY_RECTANGLE({ .color = COLOR_RED })
    ) {
        // Use CLAY_STRING directly since these are known string literals
        if (ACTIVE_PAGE == 0) {
            CLAY(CLAY_ID("NavHome"), CLAY_LAYOUT({ .padding = { 16, 8 } }), 
                CLAY_RECTANGLE({ .color = COLOR_RED_HOVER, .cornerRadius = CLAY_CORNER_RADIUS(5), .cursorPointer = true }),
                Clay_OnHover(HandleNavInteraction, 0)
            ) {
                CLAY_TEXT(CLAY_STRING("Home"), CLAY_TEXT_CONFIG({ .fontSize = 20, .fontId = FONT_ID_BODY_24, .textColor = COLOR_ORANGE, .disablePointerEvents = true }));
            }
        } else {
            CLAY(CLAY_ID("NavHome"), CLAY_LAYOUT({ .padding = { 16, 8 } }), 
                CLAY_RECTANGLE({ .color = Clay_Hovered() ? COLOR_LIGHT_HOVER : COLOR_LIGHT, .cornerRadius = CLAY_CORNER_RADIUS(5), .cursorPointer = true }),
                Clay_OnHover(HandleNavInteraction, 0)
            ) {
                CLAY_TEXT(CLAY_STRING("Home"), CLAY_TEXT_CONFIG({ .fontSize = 20, .fontId = FONT_ID_BODY_24, .textColor = COLOR_ORANGE, .disablePointerEvents = true }));
            }
        }

        if (ACTIVE_PAGE == 1) {
            CLAY(CLAY_ID("NavHabits"), CLAY_LAYOUT({ .padding = { 16, 8 } }), 
                CLAY_RECTANGLE({ .color = COLOR_RED_HOVER, .cornerRadius = CLAY_CORNER_RADIUS(5), .cursorPointer = true }),
                Clay_OnHover(HandleNavInteraction, 1)
            ) {
                CLAY_TEXT(CLAY_STRING("Habits"), CLAY_TEXT_CONFIG({ .fontSize = 20, .fontId = FONT_ID_BODY_24, .textColor = COLOR_ORANGE, .disablePointerEvents = true }));
            }
        } else {
            CLAY(CLAY_ID("NavHabits"), CLAY_LAYOUT({ .padding = { 16, 8 } }), 
                CLAY_RECTANGLE({ .color = Clay_Hovered() ? COLOR_LIGHT_HOVER : COLOR_LIGHT, .cornerRadius = CLAY_CORNER_RADIUS(5), .cursorPointer = true }),
                Clay_OnHover(HandleNavInteraction, 1)
            ) {
                CLAY_TEXT(CLAY_STRING("Habits"), CLAY_TEXT_CONFIG({ .fontSize = 20, .fontId = FONT_ID_BODY_24, .textColor = COLOR_ORANGE, .disablePointerEvents = true }));
            }
        }

        if (ACTIVE_PAGE == 2) {
            CLAY(CLAY_ID("NavTodos"), CLAY_LAYOUT({ .padding = { 16, 8 } }), 
                CLAY_RECTANGLE({ .color = COLOR_RED_HOVER, .cornerRadius = CLAY_CORNER_RADIUS(5), .cursorPointer = true }),
                Clay_OnHover(HandleNavInteraction, 2)
            ) {
                CLAY_TEXT(CLAY_STRING("Todos"), CLAY_TEXT_CONFIG({ .fontSize = 20, .fontId = FONT_ID_BODY_24, .textColor = COLOR_ORANGE, .disablePointerEvents = true }));
            }
        } else {
            CLAY(CLAY_ID("NavTodos"), CLAY_LAYOUT({ .padding = { 16, 8 } }), 
                CLAY_RECTANGLE({ .color = Clay_Hovered() ? COLOR_LIGHT_HOVER : COLOR_LIGHT, .cornerRadius = CLAY_CORNER_RADIUS(5), .cursorPointer = true }),
                Clay_OnHover(HandleNavInteraction, 2)
            ) {
                CLAY_TEXT(CLAY_STRING("Todos"), CLAY_TEXT_CONFIG({ .fontSize = 20, .fontId = FONT_ID_BODY_24, .textColor = COLOR_ORANGE, .disablePointerEvents = true }));
            }
        }

        if (ACTIVE_PAGE == 3) {
            CLAY(CLAY_ID("NavTimeline"), CLAY_LAYOUT({ .padding = { 16, 8 } }), 
                CLAY_RECTANGLE({ .color = COLOR_RED_HOVER, .cornerRadius = CLAY_CORNER_RADIUS(5), .cursorPointer = true }),
                Clay_OnHover(HandleNavInteraction, 3)
            ) {
                CLAY_TEXT(CLAY_STRING("Timeline"), CLAY_TEXT_CONFIG({ .fontSize = 20, .fontId = FONT_ID_BODY_24, .textColor = COLOR_ORANGE, .disablePointerEvents = true }));
            }
        } else {
            CLAY(CLAY_ID("NavTimeline"), CLAY_LAYOUT({ .padding = { 16, 8 } }), 
                CLAY_RECTANGLE({ .color = Clay_Hovered() ? COLOR_LIGHT_HOVER : COLOR_LIGHT, .cornerRadius = CLAY_CORNER_RADIUS(5), .cursorPointer = true }),
                Clay_OnHover(HandleNavInteraction, 3)
            ) {
                CLAY_TEXT(CLAY_STRING("Timeline"), CLAY_TEXT_CONFIG({ .fontSize = 20, .fontId = FONT_ID_BODY_24, .textColor = COLOR_ORANGE, .disablePointerEvents = true }));
            }
        }

        if (ACTIVE_PAGE == 4) {
            CLAY(CLAY_ID("NavRoutine"), CLAY_LAYOUT({ .padding = { 16, 8 } }), 
                CLAY_RECTANGLE({ .color = COLOR_RED_HOVER, .cornerRadius = CLAY_CORNER_RADIUS(5), .cursorPointer = true }),
                Clay_OnHover(HandleNavInteraction, 4)
            ) {
                CLAY_TEXT(CLAY_STRING("Routine"), CLAY_TEXT_CONFIG({ .fontSize = 20, .fontId = FONT_ID_BODY_24, .textColor = COLOR_ORANGE, .disablePointerEvents = true }));
            }
        } else {
            CLAY(CLAY_ID("NavRoutine"), CLAY_LAYOUT({ .padding = { 16, 8 } }), 
                CLAY_RECTANGLE({ .color = Clay_Hovered() ? COLOR_LIGHT_HOVER : COLOR_LIGHT, .cornerRadius = CLAY_CORNER_RADIUS(5), .cursorPointer = true }),
                Clay_OnHover(HandleNavInteraction, 4)
            ) {
                CLAY_TEXT(CLAY_STRING("Routine"), CLAY_TEXT_CONFIG({ .fontSize = 20, .fontId = FONT_ID_BODY_24, .textColor = COLOR_ORANGE, .disablePointerEvents = true }));
            }
        }
    }
}
void RenderHomePageCard(uint32_t index) {
    switch(index) {
        case 0:
            CLAY(CLAY_IDI("HomeCard", index), 
                CLAY_LAYOUT({ 
                    .sizing = { CLAY_SIZING_PERCENT(0.25f), CLAY_SIZING_FIXED(200) },
                    .padding = { 24, 24 },
                    .childGap = 16,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM
                }),
                CLAY_RECTANGLE({ 
                    .color = Clay_Hovered() ? COLOR_LIGHT_HOVER : COLOR_LIGHT,
                    .cornerRadius = CLAY_CORNER_RADIUS(10),
                    .cursorPointer = true
                }),
                CLAY_BORDER_OUTSIDE_RADIUS(2, COLOR_RED, 10),
                Clay_OnHover(HandleNavInteraction, index + 1)
            ) {
                CLAY_TEXT(CLAY_STRING("Habits"), 
                    CLAY_TEXT_CONFIG({ 
                        .fontSize = 28,
                        .fontId = FONT_ID_TITLE_36,
                        .textColor = COLOR_ORANGE,
                        .disablePointerEvents = true
                    })
                );
                
                CLAY_TEXT(CLAY_STRING("Track and maintain your daily habits for better living"), 
                    CLAY_TEXT_CONFIG({ 
                        .fontSize = 16,
                        .fontId = FONT_ID_BODY_16,
                        .textColor = COLOR_ORANGE,
                        .disablePointerEvents = true
                    })
                );
            }
            break;

        case 1:
            CLAY(CLAY_IDI("HomeCard", index), 
                CLAY_LAYOUT({ 
                    .sizing = { CLAY_SIZING_PERCENT(0.25f), CLAY_SIZING_FIXED(200) },
                    .padding = { 24, 24 },
                    .childGap = 16,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM
                }),
                CLAY_RECTANGLE({ 
                    .color = Clay_Hovered() ? COLOR_LIGHT_HOVER : COLOR_LIGHT,
                    .cornerRadius = CLAY_CORNER_RADIUS(10),
                    .cursorPointer = true
                }),
                CLAY_BORDER_OUTSIDE_RADIUS(2, COLOR_RED, 10),
                Clay_OnHover(HandleNavInteraction, index + 1)
            ) {
                CLAY_TEXT(CLAY_STRING("Todos"), 
                    CLAY_TEXT_CONFIG({ 
                        .fontSize = 28,
                        .fontId = FONT_ID_TITLE_36,
                        .textColor = COLOR_ORANGE,
                        .disablePointerEvents = true
                    })
                );
                
                CLAY_TEXT(CLAY_STRING("Manage your tasks and stay organized"), 
                    CLAY_TEXT_CONFIG({ 
                        .fontSize = 16,
                        .fontId = FONT_ID_BODY_16,
                        .textColor = COLOR_ORANGE,
                        .disablePointerEvents = true
                    })
                );
            }
            break;

        case 2:
            CLAY(CLAY_IDI("HomeCard", index), 
                CLAY_LAYOUT({ 
                    .sizing = { CLAY_SIZING_PERCENT(0.25f), CLAY_SIZING_FIXED(200) },
                    .padding = { 24, 24 },
                    .childGap = 16,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM
                }),
                CLAY_RECTANGLE({ 
                    .color = Clay_Hovered() ? COLOR_LIGHT_HOVER : COLOR_LIGHT,
                    .cornerRadius = CLAY_CORNER_RADIUS(10),
                    .cursorPointer = true
                }),
                CLAY_BORDER_OUTSIDE_RADIUS(2, COLOR_RED, 10),
                Clay_OnHover(HandleNavInteraction, index + 1)
            ) {
                CLAY_TEXT(CLAY_STRING("Timeline"), 
                    CLAY_TEXT_CONFIG({ 
                        .fontSize = 28,
                        .fontId = FONT_ID_TITLE_36,
                        .textColor = COLOR_ORANGE,
                        .disablePointerEvents = true
                    })
                );
                
                CLAY_TEXT(CLAY_STRING("View your life's journey and important moments"), 
                    CLAY_TEXT_CONFIG({ 
                        .fontSize = 16,
                        .fontId = FONT_ID_BODY_16,
                        .textColor = COLOR_ORANGE,
                        .disablePointerEvents = true
                    })
                );
            }
            break;

        case 3:
            CLAY(CLAY_IDI("HomeCard", index), 
                CLAY_LAYOUT({ 
                    .sizing = { CLAY_SIZING_PERCENT(0.25f), CLAY_SIZING_FIXED(200) },
                    .padding = { 24, 24 },
                    .childGap = 16,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM
                }),
                CLAY_RECTANGLE({ 
                    .color = Clay_Hovered() ? COLOR_LIGHT_HOVER : COLOR_LIGHT,
                    .cornerRadius = CLAY_CORNER_RADIUS(10),
                    .cursorPointer = true
                }),
                CLAY_BORDER_OUTSIDE_RADIUS(2, COLOR_RED, 10),
                Clay_OnHover(HandleNavInteraction, index + 1)
            ) {
                CLAY_TEXT(CLAY_STRING("Routine"), 
                    CLAY_TEXT_CONFIG({ 
                        .fontSize = 28,
                        .fontId = FONT_ID_TITLE_36,
                        .textColor = COLOR_ORANGE,
                        .disablePointerEvents = true
                    })
                );
                
                CLAY_TEXT(CLAY_STRING("Plan and track your daily routines"), 
                    CLAY_TEXT_CONFIG({ 
                        .fontSize = 16,
                        .fontId = FONT_ID_BODY_16,
                        .textColor = COLOR_ORANGE,
                        .disablePointerEvents = true
                    })
                );
            }
            break;

        default:
            CLAY(CLAY_IDI("HomeCard", index), 
                CLAY_LAYOUT({ 
                    .sizing = { CLAY_SIZING_PERCENT(0.25f), CLAY_SIZING_FIXED(200) },
                    .padding = { 24, 24 },
                    .childGap = 16,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM
                }),
                CLAY_RECTANGLE({ 
                    .color = Clay_Hovered() ? COLOR_LIGHT_HOVER : COLOR_LIGHT,
                    .cornerRadius = CLAY_CORNER_RADIUS(10),
                    .cursorPointer = true
                }),
                CLAY_BORDER_OUTSIDE_RADIUS(2, COLOR_RED, 10),
                Clay_OnHover(HandleNavInteraction, index + 1)
            ) {
                CLAY_TEXT(CLAY_STRING("Unknown"), 
                    CLAY_TEXT_CONFIG({ 
                        .fontSize = 28,
                        .fontId = FONT_ID_TITLE_36,
                        .textColor = COLOR_ORANGE,
                        .disablePointerEvents = true
                    })
                );
                
                CLAY_TEXT(CLAY_STRING("Unknown"), 
                    CLAY_TEXT_CONFIG({ 
                        .fontSize = 16,
                        .fontId = FONT_ID_BODY_16,
                        .textColor = COLOR_ORANGE,
                        .disablePointerEvents = true
                    })
                );
            }
            break;
    }
}


void RenderHomePage() {
    CLAY(CLAY_ID("HomeContainer"), 
        CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .padding = { 32, 32 },
            .childGap = 32,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        })
    ) {
        CLAY(CLAY_ID("HomePageTitle"),
            CLAY_LAYOUT({
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIT(0) },
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
                .padding = { 0, 32 }
            })
        ) {
            CLAY_TEXT(CLAY_STRING("Welcome to MyQuest"),
                CLAY_TEXT_CONFIG({
                    .fontSize = 48,
                    .fontId = FONT_ID_TITLE_56,
                    .textColor = COLOR_ORANGE
                })
            );
        }

        CLAY(CLAY_ID("CardGrid"), 
            CLAY_LAYOUT({ 
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
                .childGap = 32,
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER }
            })
        ) {
            for(uint32_t i = 0; i < 4; i++) {
                RenderHomePageCard(i);
            }
        }
    }
}

void RenderCurrentPage() {
    switch(ACTIVE_PAGE) {
        case 0:
            RenderHomePage();
            break;
        case 1:
            CLAY_TEXT(CLAY_STRING("Habits Page - Coming Soon"), 
                CLAY_TEXT_CONFIG({ 
                    .fontSize = 36,
                    .fontId = FONT_ID_TITLE_36,
                    .textColor = COLOR_ORANGE
                })
            );
            break;
        case 2:
            CLAY_TEXT(CLAY_STRING("Todos Page - Coming Soon"), 
                CLAY_TEXT_CONFIG({ 
                    .fontSize = 36,
                    .fontId = FONT_ID_TITLE_36,
                    .textColor = COLOR_ORANGE
                })
            );
            break;
        case 3:
            CLAY_TEXT(CLAY_STRING("Timeline Page - Coming Soon"), 
                CLAY_TEXT_CONFIG({ 
                    .fontSize = 36,
                    .fontId = FONT_ID_TITLE_36,
                    .textColor = COLOR_ORANGE
                })
            );
            break;
        case 4:
            CLAY_TEXT(CLAY_STRING("Routine Page - Coming Soon"), 
                CLAY_TEXT_CONFIG({ 
                    .fontSize = 36,
                    .fontId = FONT_ID_TITLE_36,
                    .textColor = COLOR_ORANGE
                })
            );
            break;
    }
}

Clay_RenderCommandArray CreateLayout() {
    Clay_BeginLayout();
    CLAY(CLAY_ID("OuterContainer"), 
        CLAY_LAYOUT({ 
            .layoutDirection = CLAY_TOP_TO_BOTTOM, 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() } 
        }), 
        CLAY_RECTANGLE({ .color = COLOR_LIGHT })
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