#define ROCKS_CLAY_IMPLEMENTATION
#include "rocks.h"
#include <stdio.h>
#include "quest_theme.h"

#include "config.h"
#include "components/nav.h"

#include "pages/home.h"    
#include "pages/habits.h"  
#include "pages/todos.h" 
#include "pages/timeline.h"
#include "pages/routine.h" 


#ifndef __EMSCRIPTEN__
#include "utils.h"
#endif

PageID ACTIVE_PAGE = PAGE_HOME;

// Font loading configuration
typedef struct {
    const char* path;
    int size;
    uint32_t id;
} FontConfig;

static const FontConfig FONT_CONFIGS[] = {
    {"fonts/Calistoga-Regular.ttf", 16, FONT_ID_BODY_16},
    {"fonts/Quicksand-Semibold.ttf", 56, FONT_ID_TITLE_56},
    {"fonts/Calistoga-Regular.ttf", 24, FONT_ID_BODY_24},
    {"fonts/Calistoga-Regular.ttf", 36, FONT_ID_BODY_36},
    {"fonts/Quicksand-Semibold.ttf", 36, FONT_ID_TITLE_36},
    {"fonts/Quicksand-Semibold.ttf", 24, FONT_ID_MONOSPACE_24},
    {"fonts/Calistoga-Regular.ttf", 14, FONT_ID_BODY_14},
    {"fonts/Calistoga-Regular.ttf", 18, FONT_ID_BODY_18}
};
#define FONT_CONFIG_COUNT (sizeof(FONT_CONFIGS) / sizeof(FONT_CONFIGS[0]))

static bool LoadFonts(void) {
    printf("DEBUG: Starting to load fonts...\n");
    
    bool success = true;
    uint16_t font_id;
    for (size_t i = 0; i < FONT_CONFIG_COUNT; i++) {
        font_id = Rocks_LoadFont(FONT_CONFIGS[i].path, FONT_CONFIGS[i].size, FONT_CONFIGS[i].id);
        if (font_id == UINT16_MAX) {
            printf("ERROR: Failed to load font: %s (size: %d)\n", 
                   FONT_CONFIGS[i].path, FONT_CONFIGS[i].size);
            success = false;
            break;
        }
        printf("DEBUG: Successfully loaded font %s (size: %d, id: %u)\n",
               FONT_CONFIGS[i].path, FONT_CONFIGS[i].size, font_id);
    }
    if (!success) {
        // Cleanup any fonts that were loaded
        for (uint16_t i = 0; i < FONT_CONFIG_COUNT; i++) {
            Rocks_UnloadFont(i);
        }
        return false;
    }
    printf("DEBUG: Successfully loaded all fonts\n");
    return true;
}

// Initialize pages
static void InitializePages(Rocks* rocks) {
    printf("DEBUG: Starting page initialization...\n");
    InitializeHabitsPage(rocks);
    InitializeTodosPage(rocks);
    printf("DEBUG: Pages initialized successfully\n");
}

// Cleanup pages
static void CleanupPages(Rocks* rocks) {
    printf("DEBUG: Starting cleanup...\n");
    CleanupNavIcons(rocks); 
    CleanupHomePage();
    CleanupHabitsPage(rocks);
    CleanupTodosPage(rocks);
    printf("DEBUG: Cleanup completed\n");
}


static Clay_RenderCommandArray update(Rocks* rocks, float dt) {
    Rocks_Theme theme = Rocks_GetTheme(rocks);

    Clay_BeginLayout();
    CLAY({
        .id = CLAY_ID("MainContainer"),
        .layout = {
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childGap = 20
        },
        .backgroundColor = theme.background
    }) {

        // Render the current page
        switch (ACTIVE_PAGE) {
            case PAGE_HOME: RenderHomePage(); break;
            case PAGE_HABITS: RenderHabitsPage(dt); break;
            case PAGE_TODOS: RenderTodosPage(dt); break;
            case PAGE_TIMELINE: RenderTimelinePage(); break;
            case PAGE_ROUTINE: RenderRoutinePage(); break;
            default: 
                // Handle invalid page or NUM_PAGES
                printf("WARNING: Invalid page ID encountered\n");
                break;
        }
        RenderNavigationMenu(rocks);
    }
    return Clay_EndLayout();
}

int main(void) {
    printf("DEBUG: Program starting...\n");

    // Create the Quest theme and configure Rocks
    QuestTheme theme;
    
    Rocks_Config config = {
        .window_width = 800,
        .window_height = 600,
        .window_title = "Quest",
        .arena_size = 1024 * 1024 * 64
    };

#ifdef ROCKS_USE_SDL2
    Rocks_ConfigSDL2 sdl_config = {
        .window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE,
        .renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC,
        .scale_factor = 1.0f,
        .vsync = true,
        .high_dpi = true
    };
    config.renderer_config = &sdl_config;
    printf("DEBUG: SDL2 renderer configured\n");
#endif

#ifdef ROCKS_USE_RAYLIB
    Rocks_RaylibConfig raylib_config = {
        .screen_width = 800,
        .screen_height = 600
    };
    config.renderer_config = &raylib_config;
    printf("DEBUG: Raylib renderer configured\n");
#endif

#if !defined(ROCKS_USE_SDL2) && !defined(ROCKS_USE_RAYLIB)
    printf("ERROR: No rendering backend defined. Define either ROCKS_USE_SDL2 or ROCKS_USE_RAYLIB.\n");
    return 1;
#endif

    // Create and assign theme
    theme = quest_theme_create();
    config.theme = theme.base;  // Pass the base theme to Rocks

    // Initialize Rocks
    printf("DEBUG: Initializing Rocks...\n");

    const int MAX_ELEMENTS = 16384; 
    Clay_SetMaxElementCount(MAX_ELEMENTS);

    Rocks* rocks = Rocks_Init(config);
    if (!rocks) {
        printf("ERROR: Failed to initialize Rocks\n");
        quest_theme_destroy(&theme);  // Clean up theme if Rocks init fails
        return 1;
    }
    printf("DEBUG: Rocks initialized successfully\n");

    // Load fonts
    printf("DEBUG: Starting font loading...\n");
    if (!LoadFonts()) {
        printf("ERROR: Font loading failed\n");
        quest_theme_destroy(&theme);  // Clean up theme
        Rocks_Cleanup(rocks);
        return 1;
    }
    printf("DEBUG: Fonts loaded successfully\n");

    // Initialize nav icons
    printf("DEBUG: Starting nav icons initialization...\n");
    InitializeNavIcons(rocks);
    printf("DEBUG: Nav icons initialized successfully\n");

    // Initialize pages
    printf("DEBUG: Starting page initialization...\n");
    InitializePages(rocks);
    printf("DEBUG: Pages initialized successfully\n");

    // Run the main loop
    printf("DEBUG: Starting main loop...\n");
    Rocks_Run(rocks, update);
    printf("DEBUG: Main loop ended\n");

    // Cleanup
    printf("DEBUG: Starting cleanup...\n");
    CleanupPages(rocks);
    quest_theme_destroy(&theme);  // Clean up theme
    Rocks_Cleanup(rocks);
    printf("DEBUG: Cleanup completed\n");
    printf("DEBUG: Program ending normally\n");

    return 0;
}