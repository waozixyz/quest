#include "components/nav.h"
#include "rocks.h"
#include "quest_theme.h"
#include "pages/pages.h"

typedef struct {
    const char* url;
    Clay_Dimensions dimensions;
} NavIcon;

static NavIcon NAV_ICONS[] = {
    {.url = "images/icons/home.png", .dimensions = {32, 32}},
    {.url = "images/icons/habits.png", .dimensions = {32, 32}},
    {.url = "images/icons/todos.png", .dimensions = {32, 32}},
    {.url = "images/icons/timeline.png", .dimensions = {32, 32}},
    {.url = "images/icons/routine.png", .dimensions = {32, 32}}
};

static void* nav_icon_images[5] = {NULL};

void InitializeNavIcons(Rocks* rocks) {
    for (int i = 0; i < 5; i++) {
        if (nav_icon_images[i]) {
            Rocks_UnloadImage(rocks, nav_icon_images[i]);
            nav_icon_images[i] = NULL;
        }

        nav_icon_images[i] = Rocks_LoadImage(rocks, NAV_ICONS[i].url);
        if (!nav_icon_images[i]) {
            fprintf(stderr, "Failed to load nav icon %s\n", NAV_ICONS[i].url);
            continue;
        }
    }
}

void CleanupNavIcons(Rocks* rocks) {
    for (int i = 0; i < 5; i++) {
        if (nav_icon_images[i]) {
            Rocks_UnloadImage(rocks, nav_icon_images[i]);
            nav_icon_images[i] = NULL;
        }
    }
}

void RenderNavItem(Rocks* rocks, const char* text, uint32_t pageId) {
    Rocks_Theme base_theme = Rocks_GetTheme(rocks);
    QuestThemeExtension* theme = (QuestThemeExtension*)base_theme.extension;

    bool isActive = ACTIVE_PAGE == pageId;
    bool isXSmallScreen = windowWidth < BREAKPOINT_XSMALL;
    bool isSmallScreen = windowWidth >= BREAKPOINT_XSMALL && windowWidth < BREAKPOINT_MEDIUM;
    bool isMediumScreen = windowWidth >= BREAKPOINT_MEDIUM;

    float buttonWidth = isXSmallScreen ? 60.0f : (isSmallScreen ? 80.0f : 120.0f);
    float padding = isXSmallScreen ? 4.0f : 8.0f;
    int fontSize = isMediumScreen ? 18 : 14;
    uint32_t fontId = isMediumScreen ? FONT_ID_BODY_18 : FONT_ID_BODY_14;
    float childGap = isMediumScreen ? 8.0f : 4.0f;

    Clay_TextElementConfig *text_config = CLAY_TEXT_CONFIG({ 
        .fontSize = fontSize,
        .fontId = fontId,
        .textColor = isActive ? theme->nav_text_active : theme->nav_text,
        .disablePointerEvents = true 
    });
    
    CLAY(CLAY_IDI("Nav", pageId), 
        CLAY_LAYOUT({ 
            .padding = {padding, padding},
            .childGap = childGap,
            .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
            .sizing = { CLAY_SIZING_FIXED(buttonWidth), CLAY_SIZING_FIXED(60) }
        }), 
        CLAY_RECTANGLE({ 
            .color = isActive ? theme->nav_item_active : theme->nav_item_background,
            .cornerRadius = CLAY_CORNER_RADIUS(8),
            .cursorPointer = true
        }),
        Clay_OnHover(HandleNavInteraction, pageId)
    ) {
        CLAY(CLAY_LAYOUT({ 
            .sizing = { 
                CLAY_SIZING_FIXED(32),
                CLAY_SIZING_FIXED(32)
            }
        }),
        CLAY_IMAGE({ 
            .sourceDimensions = NAV_ICONS[pageId].dimensions,
            .imageData = nav_icon_images[pageId]
        })) {}

        if (isMediumScreen) {
            Clay_String nav_text = { .chars = text, .length = strlen(text) };
            CLAY_TEXT(nav_text, text_config);
        }
    }
}

void RenderNavigationMenu(Rocks* rocks) {
    Rocks_Theme base_theme = Rocks_GetTheme(rocks);
    QuestThemeExtension* theme = (QuestThemeExtension*)base_theme.extension;
    
    bool isXSmallScreen = windowWidth < BREAKPOINT_XSMALL;

    CLAY(CLAY_ID("BottomNavigation"), 
        CLAY_LAYOUT({ 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_FIXED(isXSmallScreen ? 60.0f : 80.0f) },
            .childGap = 0,
            .padding = { isXSmallScreen ? 4.0f : 8.0f, isXSmallScreen ? 4.0f : 8.0f },
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_LEFT_TO_RIGHT
        }),
        CLAY_RECTANGLE({ 
            .color = theme->nav_background,
            .shadowEnabled = true,
            .shadowColor = theme->nav_shadow,
            .shadowOffset = {0, 2},
            .shadowBlurRadius = 8,
            .shadowSpread = 0
        })
    ) {
        RenderNavItem(rocks, "Habits", 1);
        RenderNavItem(rocks, "Todos", 2);
        RenderNavItem(rocks, "Home", 0);
        RenderNavItem(rocks, "Timeline", 3);
        RenderNavItem(rocks, "Routine", 4);
    }
}