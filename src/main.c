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

#ifdef CLAY_DESKTOP
#include <SDL.h>
#include <SDL_ttf.h>
#include "desktop/clay_sdl_renderer.h"
#endif

double windowWidth = 1024, windowHeight = 768;
uint32_t ACTIVE_PAGE = 0;
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
    printf("BeginLayout...\n");
    Clay_BeginLayout();
    
    printf("Creating outer container...\n");
    CLAY(CLAY_ID("OuterContainer"), 
        CLAY_LAYOUT({ 
            .layoutDirection = CLAY_TOP_TO_BOTTOM, 
            .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() } 
        }), 
        CLAY_RECTANGLE({ .color = COLOR_BACKGROUND })
    ) {
        printf("Rendering navigation menu...\n");
        RenderNavigationMenu();
        
        printf("Creating main content...\n");
        CLAY(CLAY_ID("MainContent"),
            CLAY_LAYOUT({ 
                .sizing = { CLAY_SIZING_GROW(), CLAY_SIZING_GROW() },
                .padding = { 16, 16 }
            })
        ) {
            printf("Rendering current page...\n");
            RenderCurrentPage();
        }
    }
    printf("EndLayout...\n");
    return Clay_EndLayout();
}

#ifdef CLAY_WASM
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
#endif

#ifdef CLAY_DESKTOP
void HandleSDLEvents(bool* running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                *running = false;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    windowWidth = event.window.data1;
                    windowHeight = event.window.data2;
                }
                break;
            case SDL_MOUSEMOTION:
                Clay_SetPointerState(
                    (Clay_Vector2){(float)event.motion.x, (float)event.motion.y},
                    (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK) != 0
                );
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                Clay_SetPointerState(
                    (Clay_Vector2){(float)event.button.x, (float)event.button.y},
                    event.type == SDL_MOUSEBUTTONDOWN
                );
                break;
        }
    }
}
#endif
int main() {
    printf("Starting application...\n");
    
#ifdef CLAY_DESKTOP
    printf("Initializing SDL...\n");
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    printf("Initializing TTF...\n");
    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF initialization failed: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    printf("Creating window...\n");
    SDL_Window* window = SDL_CreateWindow(
        "Clay App",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        windowWidth,
        windowHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    printf("Creating renderer...\n");
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    if (!renderer) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    printf("Setting up Clay...\n");
    Clay_SDL2_InitRenderer();

    printf("Loading fonts...\n");
    if (!Clay_SDL2_LoadFont(FONT_ID_BODY_16, "fonts/Quicksand-Semibold.ttf", 16) ||
        !Clay_SDL2_LoadFont(FONT_ID_TITLE_56, "fonts/Calistoga-Regular.ttf", 56) ||
        !Clay_SDL2_LoadFont(FONT_ID_BODY_24, "fonts/Quicksand-Semibold.ttf", 24) ||
        !Clay_SDL2_LoadFont(FONT_ID_BODY_36, "fonts/Quicksand-Semibold.ttf", 36) ||
        !Clay_SDL2_LoadFont(FONT_ID_TITLE_36, "fonts/Calistoga-Regular.ttf", 36) ||
        !Clay_SDL2_LoadFont(FONT_ID_MONOSPACE_24, "fonts/Calistoga-Regular.ttf", 24)) {
        fprintf(stderr, "Font loading failed\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    printf("Initializing Clay arena...\n");
    uint32_t minSize = Clay_MinMemorySize();
    void* arenaMemory = malloc(minSize);
    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(minSize, arenaMemory);

    printf("Initializing Clay...\n");
    Clay_ErrorHandler errorHandler = { .errorHandlerFunction = NULL };
    Clay_Initialize(arena, (Clay_Dimensions){windowWidth, windowHeight}, errorHandler);

    printf("Starting main loop...\n");
    
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
        Clay_SDL2_Render(renderer, commands);

        SDL_RenderPresent(renderer);
    }

    Clay_SDL2_CleanupRenderer();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
#else
    return 0;
#endif
}