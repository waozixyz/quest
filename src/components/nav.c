#include "components/nav.h"

// Icon configurations
typedef struct {
    Clay_String url;
    Clay_Dimensions dimensions;
} NavIcon;

static NavIcon NAV_ICONS[] = {
    {.url = CLAY_STRING("images/icons/home.png"), .dimensions = {32, 32}}, // Larger icons
    {.url = CLAY_STRING("images/icons/habits.png"), .dimensions = {32, 32}},
    {.url = CLAY_STRING("images/icons/todos.png"), .dimensions = {32, 32}},
    {.url = CLAY_STRING("images/icons/timeline.png"), .dimensions = {32, 32}},
    {.url = CLAY_STRING("images/icons/routine.png"), .dimensions = {32, 32}}
};

#ifndef __EMSCRIPTEN__
static SDL_Texture* nav_icon_textures[5] = {NULL};

void InitializeNavIcons(SDL_Renderer* renderer) {
    for (int i = 0; i < 5; i++) {
        if (nav_icon_textures[i]) {
            SDL_DestroyTexture(nav_icon_textures[i]);
            nav_icon_textures[i] = NULL;
        }

        SDL_Surface* surface = load_image(NAV_ICONS[i].url.chars);
        if (!surface) {
            fprintf(stderr, "Failed to load nav icon %s\n", NAV_ICONS[i].url.chars);
            continue;
        }

        nav_icon_textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }
}

void CleanupNavIcons() {
    for (int i = 0; i < 5; i++) {
        if (nav_icon_textures[i]) {
            SDL_DestroyTexture(nav_icon_textures[i]);
            nav_icon_textures[i] = NULL;
        }
    }
}
#endif

void RenderNavItem(const char* text, uint32_t pageId) {
    bool isActive = ACTIVE_PAGE == pageId;
    bool isXSmallScreen = windowWidth < BREAKPOINT_XSMALL; // Check if the screen is very small
    bool isSmallScreen = windowWidth >= BREAKPOINT_XSMALL && windowWidth < BREAKPOINT_MEDIUM; // Check if the screen is small
    bool isMediumScreen = windowWidth >= BREAKPOINT_MEDIUM; // Check if the screen is medium or larger

    // Adjust button width and padding based on screen size
    float buttonWidth = isXSmallScreen ? 60.0f : (isSmallScreen ? 80.0f : 120.0f); // Wider buttons on larger screens
    float padding = isXSmallScreen ? 4.0f : 8.0f; // Smaller padding on very small screens

    // Adjust text size and font ID based on screen size
    int fontSize = isMediumScreen ? 18 : 14; // Larger text on medium screens
    uint32_t fontId = isMediumScreen ? FONT_ID_BODY_18 : FONT_ID_BODY_14; // Corresponding font ID

    // Adjust spacing between icon and text based on screen size
    float childGap = isMediumScreen ? 8.0f : 4.0f; // More spacing between icon and text on medium screens

    Clay_TextElementConfig *text_config = CLAY_TEXT_CONFIG({ 
        .fontSize = fontSize, // Dynamic text size
        .fontId = fontId, // Dynamic font ID
        .textColor = isActive ? COLOR_NAV_ITEM_TEXT_ACTIVE : COLOR_NAV_ITEM_TEXT, // Highlight active text
        .disablePointerEvents = true 
    });
    
    CLAY(CLAY_IDI("Nav", pageId), 
        CLAY_LAYOUT({ 
            .padding = {padding, padding}, // Dynamic padding
            .childGap = childGap, // Dynamic gap between icon and text
            .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
            .sizing = { CLAY_SIZING_FIXED(buttonWidth), CLAY_SIZING_FIXED(60) } // Dynamic width for each nav item
        }), 
        CLAY_RECTANGLE({ 
            .color = isActive ? COLOR_NAV_ITEM_BACKGROUND_ACTIVE : COLOR_NAV_ITEM_BACKGROUND, // Highlight active background
            .cornerRadius = CLAY_CORNER_RADIUS(8),
            .cursorPointer = true
        }),
        Clay_OnHover(HandleNavInteraction, pageId)
    ) {
        // Always show icons in the bottom nav bar
        CLAY(CLAY_LAYOUT({ 
            .sizing = { 
                CLAY_SIZING_FIXED(32), // Larger icons
                CLAY_SIZING_FIXED(32)
            }
        }),
        #ifdef __EMSCRIPTEN__
        CLAY_IMAGE({ 
            .sourceDimensions = NAV_ICONS[pageId].dimensions,
            .sourceURL = NAV_ICONS[pageId].url
        })
        #else
        CLAY_IMAGE({ 
            .sourceDimensions = NAV_ICONS[pageId].dimensions,
            .imageData = nav_icon_textures[pageId]
        })
        #endif
        ) {}

        // Show text below the icon only if the screen is medium or larger
        if (isMediumScreen) {
            Clay_String nav_text = { .chars = text, .length = strlen(text) };
            CLAY_TEXT(nav_text, text_config);
        }
    }
}

void RenderNavigationMenu() {
    bool isXSmallScreen = windowWidth < BREAKPOINT_XSMALL; // Check if the screen is very small

    CLAY(CLAY_ID("BottomNavigation"), 
        CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIXED(isXSmallScreen ? 60.0f : 80.0f) }, // Shorter bar on very small screens
            .childGap = 0, // No gap between items
            .padding = { isXSmallScreen ? 4.0f : 8.0f, isXSmallScreen ? 4.0f : 8.0f }, // Smaller padding on very small screens
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_LEFT_TO_RIGHT // Horizontal layout
        }),
        CLAY_RECTANGLE({ 
            .color = COLOR_NAV_BACKGROUND,
            .shadowEnabled = true, // Enable shadow
            .shadowColor = (Clay_Color){0, 0, 0, 50}, // Semi-transparent black
            .shadowOffset = {0, 2}, // Shadow offset (x, y)
            .shadowBlurRadius = 8,  // Blur radius
            .shadowSpread = 0       // Shadow spread
        })
    ) {
        RenderNavItem("Habits", 1);
        RenderNavItem("Todos", 2);
        RenderNavItem("Home", 0);
        RenderNavItem("Timeline", 3);
        RenderNavItem("Routine", 4);
    }
}