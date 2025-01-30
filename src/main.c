#define ROCKS_USE_SDL2 1
#define ROCKS_CLAY_IMPLEMENTATION

#include "rocks.h"
#include "app.h"
#include "pages/pages.h"
#include <stdio.h>
#include "quest_theme.h"


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
static bool load_fonts(void) {
    printf("DEBUG: Starting to load fonts...\n");
    
    bool success = true;
    uint16_t font_id;

    for (size_t i = 0; i < FONT_CONFIG_COUNT; i++) {
        font_id = rocks_load_font(FONT_CONFIGS[i].path, FONT_CONFIGS[i].size, FONT_CONFIGS[i].id);
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
            rocks_unload_font(i);
        }
        return false;
    }

    printf("DEBUG: Successfully loaded all fonts\n");
    return true;
}

// Initialize pages
static void initialize_pages(Rocks* rocks) {
    printf("DEBUG: Starting page initialization...\n");
    InitializeHabitsPage(rocks);
    InitializeTodosPage(rocks);
    printf("DEBUG: Pages initialized successfully\n");
}

// Cleanup pages
static void cleanup_pages(Rocks* rocks) {
    printf("DEBUG: Starting cleanup...\n");
    CleanupNavIcons(rocks); 
    CleanupHomePage();
    CleanupHabitsPage(rocks);
    CleanupTodosPage(rocks);
    printf("DEBUG: Cleanup completed\n");
}

// Update callback for Rocks
static Clay_RenderCommandArray update(Rocks* rocks, float dt) {
    RocksTheme theme = rocks_get_theme(rocks);

    CLAY(CLAY_ID("MainContainer"), 
        CLAY_LAYOUT({
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
            .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childGap = 20
        }),
        CLAY_RECTANGLE({ .color = theme.background })
    ) {
        // Render the current page
        switch (ACTIVE_PAGE) {
            case PAGE_HOME: RenderHomePage(); break;
            case PAGE_HABITS: RenderHabitsPage(); break;
            case PAGE_TODOS: RenderTodosPage(); break;
            case PAGE_TIMELINE: RenderTimelinePage(); break;
            case PAGE_ROUTINE: RenderRoutinePage(); break;
        }

        // Render navigation menu
        RenderNavigationMenu(rocks);
    }

    return Clay_EndLayout();
}

int main(void) {
    printf("DEBUG: Program starting...\n");

    // Configure Rocks
    RocksSDL2Config sdl_config = {
        .window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE,
        .renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC,
        .scale_factor = 1.0f,
        .vsync = true,
        .high_dpi = true
    };

    printf("DEBUG: SDL config initialized\n");

    // Create the Quest theme
    QuestTheme theme = quest_theme_create();

    RocksConfig config = {
        .window_width = 800,
        .window_height = 600,
        .window_title = "Quest",
        .renderer_config = &sdl_config,
        .theme = theme.base  // Pass the base theme to Rocks
    };
    printf("DEBUG: Rocks config initialized\n");

    // Initialize Rocks
    printf("DEBUG: Initializing Rocks...\n");
    Rocks* rocks = rocks_init(config);
    if (!rocks) {
        printf("ERROR: Failed to initialize Rocks: %s\n", SDL_GetError());
        return 1;
    }
    printf("DEBUG: Rocks initialized successfully\n");

    // Load fonts
    printf("DEBUG: Starting font loading...\n");
    if (!load_fonts()) {
        printf("ERROR: Font loading failed\n");
        rocks_cleanup(rocks);
        return 1;
    }
    printf("DEBUG: Fonts loaded successfully\n");

    // Initialize nav icons
    printf("DEBUG: Starting nav icons initialization...\n");
    InitializeNavIcons(rocks);
    printf("DEBUG: Nav icons initialized successfully\n");

    // Initialize pages
    printf("DEBUG: Starting page initialization...\n");
    initialize_pages(rocks);
    printf("DEBUG: Pages initialized successfully\n");

    // Run the main loop
    printf("DEBUG: Starting main loop...\n");
    rocks_run(rocks, update);
    printf("DEBUG: Main loop ended\n");

    // Cleanup
    printf("DEBUG: Starting cleanup...\n");
    cleanup_pages(rocks);
    rocks_cleanup(rocks);
    printf("DEBUG: Cleanup completed\n");

    printf("DEBUG: Program ending normally\n");
    return 0;
}