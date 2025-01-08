#define CLAY_IMPLEMENTATION
#include "clay_extensions.h"
#include "../vendor/clay/clay.h"

#include "styles.h"
#include "components/nav.h"
#include "components/text_input.h"
#include "pages/home.h"
#include "pages/habits.h"
#include "pages/todos.h"
#include "pages/timeline.h"
#include "pages/routine.h"
#include "utils.h"
#include <unistd.h> 

#ifndef __EMSCRIPTEN__
#include <SDL.h>
#include <SDL_ttf.h>
#include "renderers/clay_sdl_renderer.h"

static bool isScrollDragging = false;
static int scrollDragStartY = 0;
static int scrollDragStartX = 0;
static Clay_Vector2 initialScrollPosition = {0, 0};
static bool isScrollThumbDragging = false;

#endif


double windowWidth = 1024, windowHeight = 768;
float globalScalingFactor = 1.0f;
uint32_t ACTIVE_PAGE = 0;
uint32_t ACTIVE_RENDERER_INDEX = 0;
bool pages_initialized = false;

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
const Clay_Color COLOR_CURSOR = (Clay_Color){255, 107, 151, 255};           // Using COLOR_ACCENT for cursor


const int DEFAULT_PADDING = 32;

void HandleNavInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        ACTIVE_PAGE = (uint32_t)userData;
    }
}

#ifdef __EMSCRIPTEN__

void InitializePages() {
    if (pages_initialized) return;
    SDL_Log("Initializing pages...\n");
    InitializeHabitsPage();
    InitializeTodosPage();
    pages_initialized = true;
    SDL_Log("Pages initialized\n");
}

#else

void InitializePages(SDL_Renderer* renderer) {
    if (pages_initialized) return;
    
    SDL_Log("Initializing pages...\n");
    InitializeHabitsPage();
    InitializeTodosPage(renderer);
    pages_initialized = true;
    SDL_Log("Pages initialized\n");
}



#endif
void RenderCurrentPage() {
    switch(ACTIVE_PAGE) {
        case 0: RenderHomePage(); break;
        case 1: RenderHabitsPage(); break;
        case 2: RenderTodosPage(); break;
        case 3: RenderTimelinePage(); break;
        case 4: RenderRoutinePage(); break;
    }
}
void CleanupPages() {
    SDL_Log("Cleaning up pages...\n");
    CleanupHabitsPage();
    CleanupTodosPage();
    // Add other page cleanups here as needed:
    // CleanupTimelinePage();
    // etc.
    pages_initialized = false;
    SDL_Log("Pages cleaned up\n");
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
                .padding = { 16, 16 }
            })
        ) {
            RenderCurrentPage();
        }
    }

    Clay_RenderCommandArray commands = Clay_EndLayout();
    

    // Additional safety checks
    if (commands.length > 0 && !commands.internalArray) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "ERROR: Render commands array is NULL despite having length\n");
    }

    return commands;
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

#ifndef __EMSCRIPTEN__
void HandlePageInput(InputEvent event) {
    switch(ACTIVE_PAGE) {
        case 0: /* HandleHomePageInput(event); */ break;
        case 1: HandleHabitsPageInput(event); break;
        case 2: HandleTodosPageInput(event); break;
        case 3: /* HandleTimelinePageInput(event); */ break;
        case 4: /* HandleRoutinePageInput(event); */ break;
    }
}


void HandleSDLEvents(bool* running) {
    SDL_Event event;
    static float last_time = 0;
    float current_time = SDL_GetTicks() / 1000.0f;
    float delta_time = current_time - last_time;
    last_time = current_time;

    InputEvent input_event = {0};
    input_event.delta_time = delta_time;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                *running = false;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    windowWidth = event.window.data1 / globalScalingFactor;
                    windowHeight = event.window.data2 / globalScalingFactor;
                    Clay_SetLayoutDimensions((Clay_Dimensions){windowWidth, windowHeight});
                }
                break;
            case SDL_MOUSEWHEEL:
                {
                    Clay_Vector2 scrollDelta = {
                        event.wheel.x * 30.0f / globalScalingFactor, // Scale scroll delta
                        event.wheel.y * 30.0f / globalScalingFactor
                    };
        
                    Clay_UpdateScrollContainers(true, scrollDelta, delta_time);
                }
                break;
            case SDL_MOUSEMOTION:
                Clay_SetPointerState(
                    (Clay_Vector2){
                        (float)event.motion.x / globalScalingFactor, 
                        (float)event.motion.y / globalScalingFactor
                    },
                    (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK) != 0
                );
                
                if (isScrollThumbDragging || isScrollDragging) {
                    // Try both scroll containers
                    Clay_ScrollContainerData calendarScrollData = Clay_GetScrollContainerData(
                        Clay_GetElementId(CLAY_STRING("CalendarScrollContainer"))
                    );
                    Clay_ScrollContainerData homeScrollData = Clay_GetScrollContainerData(
                        Clay_GetElementId(CLAY_STRING("HomeScrollContainer"))
                    );
                    
                    Clay_ScrollContainerData* scrollData = 
                        calendarScrollData.found ? &calendarScrollData : 
                        (homeScrollData.found ? &homeScrollData : NULL);
                    
                    if (scrollData) {
                        if (isScrollThumbDragging) {
                            // Calculate scroll position based on mouse movement
                            float scrollableHeight = scrollData->contentDimensions.height - scrollData->scrollContainerDimensions.height;
                            float dragDelta = (event.motion.y / globalScalingFactor) - (scrollDragStartY / globalScalingFactor);
                            float scrollRatio = dragDelta / scrollData->scrollContainerDimensions.height;
                            float newScrollY = initialScrollPosition.y - (scrollRatio * scrollableHeight);
                            
                            // Clamp scroll position
                            newScrollY = CLAY__MIN(0, CLAY__MAX(
                                newScrollY, 
                                -scrollableHeight
                            ));
                            
                            *scrollData->scrollPosition = (Clay_Vector2){
                                scrollData->scrollPosition->x,
                                newScrollY
                            };
                        } else if (isScrollDragging) {
                            // Direct 1:1 movement for more natural feel
                            float deltaY = (scrollDragStartY - event.motion.y) / globalScalingFactor;
                            float newScrollY = initialScrollPosition.y - deltaY;
                            
                            // Clamp scroll position
                            float scrollableHeight = scrollData->contentDimensions.height - scrollData->scrollContainerDimensions.height;
                            newScrollY = CLAY__MIN(0, CLAY__MAX(
                                newScrollY, 
                                -scrollableHeight
                            ));
                            
                            *scrollData->scrollPosition = (Clay_Vector2){
                                scrollData->scrollPosition->x,
                                newScrollY
                            };

                            // Update drag start position for smooth continuous scrolling
                            scrollDragStartY = event.motion.y;
                            initialScrollPosition = *scrollData->scrollPosition;
                        }
                    }
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                Clay_SetPointerState(
                    (Clay_Vector2){
                        (float)event.button.x / globalScalingFactor, 
                        (float)event.button.y / globalScalingFactor
                    },
                    event.type == SDL_MOUSEBUTTONDOWN
                );

                if (event.button.button == SDL_BUTTON_LEFT) {
                    // Try both scroll containers
                    Clay_ElementId calendarScrollContainerId = Clay_GetElementId(CLAY_STRING("CalendarScrollContainer"));
                    Clay_ElementId homeScrollContainerId = Clay_GetElementId(CLAY_STRING("HomeScrollContainer"));
                    
                    Clay_ScrollContainerData calendarScrollData = Clay_GetScrollContainerData(calendarScrollContainerId);
                    Clay_ScrollContainerData homeScrollData = Clay_GetScrollContainerData(homeScrollContainerId);
                    
                    Clay_ScrollContainerData* scrollData = 
                        calendarScrollData.found ? &calendarScrollData : 
                        (homeScrollData.found ? &homeScrollData : NULL);
                    
                    Clay_ElementId scrollContainerId = 
                        calendarScrollData.found ? calendarScrollContainerId : 
                        (homeScrollData.found ? homeScrollContainerId : (Clay_ElementId){0});
                    
                    if (scrollData) {
                        Clay_LayoutElementHashMapItem* hashMapItem = Clay__GetHashMapItem(scrollContainerId.id);
                        if (hashMapItem) {
                            Clay_BoundingBox containerBox = hashMapItem->boundingBox;
                            
                            // Calculate scroll thumb position and size
                            float viewportSize = containerBox.height;
                            float contentSize = scrollData->contentDimensions.height;
                            float thumbSize = (viewportSize / contentSize) * viewportSize;
                            float thumbPosition = (-scrollData->scrollPosition->y / contentSize) * viewportSize;
                            
                            // Scale mouse coordinates
                            float scaledMouseX = event.button.x / globalScalingFactor;
                            float scaledMouseY = event.button.y / globalScalingFactor;

                            // Check if click is on the scroll thumb
                            if (scaledMouseX >= containerBox.x + containerBox.width - 10 && 
                                scaledMouseX <= containerBox.x + containerBox.width &&
                                scaledMouseY >= containerBox.y + thumbPosition &&
                                scaledMouseY <= containerBox.y + thumbPosition + thumbSize) {
                                isScrollThumbDragging = true;
                                scrollDragStartY = event.button.y;
                                initialScrollPosition = *scrollData->scrollPosition;
                            }
                            // Regular container drag check
                            else if (scaledMouseX >= containerBox.x && 
                                scaledMouseX <= containerBox.x + containerBox.width &&
                                scaledMouseY >= containerBox.y && 
                                scaledMouseY <= containerBox.y + containerBox.height) {
                                isScrollDragging = true;
                                scrollDragStartY = event.button.y;
                                scrollDragStartX = event.button.x;
                                initialScrollPosition = *scrollData->scrollPosition;
                            }
                        }
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (isScrollThumbDragging || isScrollDragging) {
                    isScrollThumbDragging = false;
                    isScrollDragging = false;
                }
                Clay_SetPointerState(
                    (Clay_Vector2){(float)event.button.x, (float)event.button.y},
                    false
                );
                break;
            case SDL_FINGERDOWN: {
                SDL_Event mouseEvent;  // Declare outside the inner blocks
                Clay_SetPointerState(
                    (Clay_Vector2){
                        event.tfinger.x * windowWidth / globalScalingFactor, 
                        event.tfinger.y * windowHeight / globalScalingFactor
                    },
                    true
                );
                // Simulate mouse button down
                mouseEvent.type = SDL_MOUSEBUTTONDOWN;
                mouseEvent.button.x = event.tfinger.x * windowWidth;
                mouseEvent.button.y = event.tfinger.y * windowHeight;
                mouseEvent.button.button = SDL_BUTTON_LEFT;
                SDL_PushEvent(&mouseEvent);
                break;
            }

            case SDL_FINGERUP: {
                SDL_Event mouseEvent;  // Reuse the declaration
                Clay_SetPointerState(
                    (Clay_Vector2){
                        event.tfinger.x * windowWidth / globalScalingFactor, 
                        event.tfinger.y * windowHeight / globalScalingFactor
                    },
                    false
                );
                // Simulate mouse button up
                mouseEvent.type = SDL_MOUSEBUTTONUP;
                mouseEvent.button.x = event.tfinger.x * windowWidth;
                mouseEvent.button.y = event.tfinger.y * windowHeight;
                mouseEvent.button.button = SDL_BUTTON_LEFT;
                SDL_PushEvent(&mouseEvent);
                break;
            }

            case SDL_FINGERMOTION: {
                SDL_Event mouseEvent;  // Reuse the declaration
                Clay_SetPointerState(
                    (Clay_Vector2){
                        event.tfinger.x * windowWidth / globalScalingFactor, 
                        event.tfinger.y * windowHeight / globalScalingFactor
                    },
                    true
                );
                // Simulate mouse motion
                mouseEvent.type = SDL_MOUSEMOTION;
                mouseEvent.motion.x = event.tfinger.x * windowWidth;
                mouseEvent.motion.y = event.tfinger.y * windowHeight;
                SDL_PushEvent(&mouseEvent);
                break;
            }
            case SDL_KEYDOWN:
                input_event.isTextInput = false;
                if (event.key.keysym.sym == SDLK_BACKSPACE) {
                    input_event.key = '\b';
                } else if (event.key.keysym.sym == SDLK_LEFT) {
                    input_event.key = 0x25;
                } else if (event.key.keysym.sym == SDLK_RIGHT) {
                    input_event.key = 0x27;
                }
                HandlePageInput(input_event);
                break;

            case SDL_TEXTINPUT:
                input_event.isTextInput = true;
                strncpy(input_event.text, event.text.text, sizeof(input_event.text) - 1);
                HandlePageInput(input_event);
                break;
        }
    }

    // Update blink timer
    input_event.isTextInput = false;
    input_event.key = 0;
    HandlePageInput(input_event);
}

#endif


void RunGameLoop(SDL_Window* window, SDL_Renderer* renderer) {    
    SDL_Log("RunGameLoop started\n");

    SDL_Log("Initializing SDL2 renderer...\n");
    Clay_SDL2_InitRenderer(renderer);
    Clay_SDL2_SetRenderScale(globalScalingFactor);  
    SDL_Log("SDL2 renderer initialized\n");

    
    if (!load_font(FONT_ID_BODY_16, "Quicksand-Semibold.ttf", 16) ||
        !load_font(FONT_ID_TITLE_56, "Calistoga-Regular.ttf", 56) ||
        !load_font(FONT_ID_BODY_24, "Quicksand-Semibold.ttf", 24) ||
        !load_font(FONT_ID_BODY_36, "Quicksand-Semibold.ttf", 36) ||
        !load_font(FONT_ID_TITLE_36, "Calistoga-Regular.ttf", 36) ||
        !load_font(FONT_ID_MONOSPACE_24, "Calistoga-Regular.ttf", 24)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load one or more fonts");
        TTF_Quit();
        return;
    }


    uint32_t minSize = Clay_MinMemorySize();
    uint32_t mobileMultiplier = 2;  // Double the memory on mobile
    uint32_t recommendedSize = minSize + (minSize * mobileMultiplier);

    void* arenaMemory = malloc(recommendedSize);
    if (!arenaMemory) {
        SDL_Log("Failed to allocate %u bytes for Clay arena\n", recommendedSize);
        TTF_Quit();
        return;
    }
    SDL_Log("Successfully allocated %u bytes for Clay arena\n", recommendedSize);

    memset(arenaMemory, 0, recommendedSize);
    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(recommendedSize, arenaMemory);
    Clay_ErrorHandler errorHandler = { .errorHandlerFunction = NULL };
    Clay_Initialize(arena, (Clay_Dimensions){windowWidth, windowHeight}, errorHandler);

    InitializeNavIcons(renderer);
    InitializePages(renderer);

    // Main Game Loop
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
    Clay_SDL2_CleanupRenderer();
    free(arenaMemory);
    TTF_Quit();
}


#if defined(__EMSCRIPTEN__)
int main() {
   return 0;
}

#elif defined(CLAY_MOBILE)
int SDL_main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);

    // Get display info
    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);

    // Get screen DPI
    float ddpi, hdpi, vdpi;
    if (SDL_GetDisplayDPI(0, &ddpi, &hdpi, &vdpi) != 0) {
        // Default to standard DPI if we can't get it
        ddpi = 160.0f;
        hdpi = 160.0f;
        vdpi = 160.0f;
    }

    // Calculate density scale and update global scaling
    float densityScale = ddpi / 160.0f;  // 160 DPI is baseline for Android
    globalScalingFactor = densityScale;

    // Calculate window dimensions
    windowWidth = displayMode.w / densityScale;
    windowHeight = displayMode.h / densityScale;

    SDL_Log("Mobile display setup: dimensions=%dx%d, dpi=%f, scale=%f", 
            displayMode.w, displayMode.h, ddpi, globalScalingFactor);


    if (TTF_Init() == -1) {
        SDL_Log("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
    }

    char *env_str = SDL_getenv("MAGICK_MEMORY_LIMIT");
    if (!env_str) {
        SDL_setenv("MAGICK_MEMORY_LIMIT", "512MB", 1);
    }

    SDL_Log("TTF initialized\n");


    SDL_Window* window = SDL_CreateWindow(
        "myQuest", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        displayMode.w,
        displayMode.h,
        SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN
    );

    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Window creation failed: %s", SDL_GetError());
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
        
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Renderer creation failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        return -1;
    }

    RunGameLoop(window, renderer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

#else
int main() {
   if (SDL_Init(SDL_INIT_VIDEO) < 0) {
       SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL initialization failed: %s\n", SDL_GetError());
       return 1;
   }

   if (TTF_Init() < 0) {
       SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF initialization failed: %s\n", TTF_GetError());
       SDL_Quit();
       return 1;
   }

   SDL_Window* window = SDL_CreateWindow(
       "Clay App",
       SDL_WINDOWPOS_UNDEFINED,
       SDL_WINDOWPOS_UNDEFINED,
       windowWidth,
       windowHeight,
       SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
   );

   if (!window) {
       SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window creation failed: %s\n", SDL_GetError());
       TTF_Quit();
       SDL_Quit();
       return 1;
   }

   SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
       SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

   if (!renderer) {
       SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer creation failed: %s\n", SDL_GetError());
       SDL_DestroyWindow(window);
       TTF_Quit();
       SDL_Quit();
       return 1;
   }

   RunGameLoop(window, renderer);

   SDL_DestroyRenderer(renderer);
   SDL_DestroyWindow(window);
   TTF_Quit();
   SDL_Quit();
   return 0;
}
#endif