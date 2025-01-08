#include "components/nav.h"

// Icon configurations
typedef struct {
    Clay_String url;
    Clay_Dimensions dimensions;
} NavIcon;

static NavIcon NAV_ICONS[] = {
    {.url = CLAY_STRING("icons/home.png"), .dimensions = {24, 24}},
    {.url = CLAY_STRING("icons/habits.png"), .dimensions = {24, 24}},
    {.url = CLAY_STRING("icons/todos.png"), .dimensions = {24, 24}},
    {.url = CLAY_STRING("icons/timeline.png"), .dimensions = {24, 24}},
    {.url = CLAY_STRING("icons/routine.png"), .dimensions = {24, 24}}
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
    bool showIcons = windowWidth < mobileBreakpoint;

    Clay_TextElementConfig *text_config = CLAY_TEXT_CONFIG({ 
        .fontSize = 20,
        .fontId = FONT_ID_BODY_24,
        .textColor = COLOR_TEXT,
        .disablePointerEvents = true 
    });
    
    CLAY(CLAY_IDI("Nav", pageId), 
        CLAY_LAYOUT({ 
            .padding = showIcons ? (Clay_Padding){8, 8} : (Clay_Padding){16, 8},
            .childGap = 8,
            .childAlignment = {.x = CLAY_ALIGN_X_CENTER }
    }), 
        CLAY_RECTANGLE({ 
            .color = isActive ? COLOR_PRIMARY : 
                     (Clay_Hovered() ? COLOR_PRIMARY_HOVER : COLOR_PANEL),
            .cornerRadius = CLAY_CORNER_RADIUS(5),
            .cursorPointer = true
        }),
        Clay_OnHover(HandleNavInteraction, pageId)
    ) {
        if (showIcons) {
            CLAY(CLAY_LAYOUT({ 
                .sizing = { 
                    CLAY_SIZING_FIXED(24), 
                    CLAY_SIZING_FIXED(24)
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
        } else {
            Clay_String nav_text = { .chars = text, .length = strlen(text) };
            CLAY_TEXT(nav_text, text_config);
        }
    }
}

void RenderNavigationMenu() {
    CLAY(CLAY_ID("TopNavigation"), 
        CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIXED(60) },
            .childGap = 16,
            .padding = { 16, 0 },
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER, .x = CLAY_ALIGN_X_CENTER }
        }),
        CLAY_RECTANGLE({ .color = COLOR_SECONDARY })
    ) {
        RenderNavItem("Home", 0);
        RenderNavItem("Habits", 1);
        RenderNavItem("Todos", 2);
        RenderNavItem("Timeline", 3);
        RenderNavItem("Routine", 4);
    }
}
