#include "app.h"

#include "clay_extensions.h"
#include "clay.h"
bool pages_initialized = false;

#ifdef __EMSCRIPTEN__
void InitializePages() {
    if (pages_initialized) return;
    InitializeHabitsPage();
    InitializeTodosPage();
    pages_initialized = true;
}

void measureTextFunction(Clay_String *text, Clay_TextElementConfig *config) {}
void queryScrollOffsetFunction(Clay_ElementId elementId) {}

#else
void InitializePages(SDL_Renderer* renderer) {
    if (pages_initialized) return;
    SDL_Log("Initializing pages...\n");
    InitializeHabitsPage(renderer);
    InitializeTodosPage(renderer);
    pages_initialized = true;
    SDL_Log("Pages initialized\n");
}
#endif

typedef enum {
    PAGE_HOME,
    PAGE_HABITS,
    PAGE_TODOS,
    PAGE_TIMELINE,
    PAGE_ROUTINE,
    NUM_PAGES
} PageID;


void HandlePageInput(InputEvent event) {
    switch (ACTIVE_PAGE) {
        case PAGE_HOME: /* HandleHomePageInput(event); */ break;
        case PAGE_HABITS: HandleHabitsPageInput(event); break;
        case PAGE_TODOS: HandleTodosPageInput(event); break;
        case PAGE_TIMELINE: /* HandleTimelinePageInput(event); */ break;
        case PAGE_ROUTINE: /* HandleRoutinePageInput(event); */ break;
    }
}
void RenderCurrentPage() {
    switch(ACTIVE_PAGE) {
        case PAGE_HOME: RenderHomePage(); break;
        case PAGE_HABITS: RenderHabitsPage(); break;
        case PAGE_TODOS: RenderTodosPage(); break;
        case PAGE_TIMELINE: RenderTimelinePage(); break;
        case PAGE_ROUTINE: RenderRoutinePage(); break;
    }
}

void CleanupPages() {
    #ifndef __EMSCRIPTEN__
    SDL_Log("Cleaning up pages...\n");
    #endif
    CleanupHabitsPage();
    CleanupTodosPage();
    pages_initialized = false;
}

Clay_RenderCommandArray CreateLayout() {
    #ifdef __EMSCRIPTEN__
    if (!pages_initialized) {
        InitializePages();
    }
    #endif

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
            })
        ) {
            RenderCurrentPage();
        }
    }

    Clay_RenderCommandArray commands = Clay_EndLayout();
    return commands;
}

#ifndef __EMSCRIPTEN__
void RunGameLoop(SDL_Window* window, SDL_Renderer* renderer) {    
    SDL_Log("RunGameLoop started\n");

    InitializeSDL(window, renderer);

    uint32_t minSize = Clay_MinMemorySize();
    uint32_t mobileMultiplier = 2;
    uint32_t recommendedSize = minSize + (minSize * mobileMultiplier);

    void* arenaMemory = malloc(recommendedSize);
    if (!arenaMemory) {
        SDL_Log("Failed to allocate %u bytes for Clay arena\n", recommendedSize);
        CleanupSDL();
        return;
    }
    SDL_Log("Successfully allocated %u bytes for Clay arena\n", recommendedSize);

    memset(arenaMemory, 0, recommendedSize);
    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(recommendedSize, arenaMemory);
    Clay_ErrorHandler errorHandler = { .errorHandlerFunction = NULL };
    Clay_Initialize(arena, (Clay_Dimensions){windowWidth, windowHeight}, errorHandler);

    InitializeNavIcons(renderer);
    InitializePages(renderer);

    bool running = true;
    while (running) {
        HandleSDLEvents(&running);

        SDL_SetRenderDrawColor(renderer, 
            COLOR_BACKGROUND.r, 
            COLOR_BACKGROUND.g, 
            COLOR_BACKGROUND.b, 
            COLOR_BACKGROUND.a
        );
        SDL_RenderClear(renderer);

        Clay_SetLayoutDimensions((Clay_Dimensions){windowWidth, windowHeight});
        Clay_RenderCommandArray commands = CreateLayout();

        if (commands.length == 0) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "WARNING: No render commands generated\n");
        }

        Clay_SDL2_Render(renderer, commands);
        SDL_RenderPresent(renderer);

    }

    CleanupPages();
    CleanupNavIcons();
    CleanupSDL();
    free(arenaMemory);
}
#endif