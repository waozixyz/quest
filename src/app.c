#include "app.h"
#include "styles.h"
#include "config.h"
#include "components/nav.h"
#include "pages/home.h"
#include "pages/habits.h"
#include "pages/todos.h"
#include "pages/timeline.h"
#include "pages/routine.h"
#include "utils.h"


#include "clay_extensions.h"
#define CLAY_IMPLEMENTATION
#include "../vendor/clay/clay.h"

#ifndef __EMSCRIPTEN__
#include "renderers/clay_sdl_renderer.h"
#include <SDL.h>
#include <SDL_ttf.h>
#endif


bool pages_initialized = false;

#ifdef __EMSCRIPTEN__
void InitializePages() {
    if (pages_initialized) return;
    InitializeHabitsPage();
    InitializeTodosPage();
    pages_initialized = true;
}
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

    // Get all active scroll containers at start of frame
    Clay_ScrollContainerData activeScrollContainers[16] = {0};  // Adjust size as needed
    Clay_ElementId activeScrollContainerIds[16] = {0};
    int activeScrollCount = 0;

    // Get all pointer-over elements and find scroll containers
    for (uint32_t i = 0; i < Clay__pointerOverIds.length && activeScrollCount < 16; i++) {
        Clay_ElementId elementId = Clay__pointerOverIds.internalArray[i];
        Clay_ScrollContainerData tempData = Clay_GetScrollContainerData(elementId);
        
        if (tempData.found) {
            activeScrollContainers[activeScrollCount] = tempData;
            activeScrollContainerIds[activeScrollCount] = elementId;
            activeScrollCount++;
        }
    }

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

            case SDL_MOUSEWHEEL: {
                float scrollMultiplier = 15.0f / globalScalingFactor;
                Clay_Vector2 scrollDelta = {0, 0};

                // Find topmost scroll container that's active
                Clay_ScrollContainerData* activeScroll = NULL;
                for (int i = 0; i < activeScrollCount; i++) {
                    activeScroll = &activeScrollContainers[i];
                    break;  // Use first (topmost) container
                }

                if (activeScroll) {
                    bool preferHorizontal = (activeScroll->contentDimensions.width > activeScroll->contentDimensions.height) ||
                                          (activeScroll->config.horizontal && !activeScroll->config.vertical);

                    if (preferHorizontal) {
                        scrollDelta.x = event.wheel.y * scrollMultiplier;
                    } else {
                        scrollDelta.y = event.wheel.y * scrollMultiplier;
                    }
                }

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
                
                if (isScrollThumbDragging || isHorizontalScrollThumbDragging || isScrollDragging) {
                    Clay_ScrollContainerData* scrollData = NULL;

                    // Find the active scroll container
                    for (int i = 0; i < activeScrollCount; i++) {
                        scrollData = &activeScrollContainers[i];
                        break;
                    }

                    if (scrollData) {

                        if (isScrollThumbDragging) {
                            float scrollableHeight = scrollData->contentDimensions.height - scrollData->scrollContainerDimensions.height;
                            float dragDelta = (event.motion.y / globalScalingFactor) - (scrollDragStartY / globalScalingFactor);
                            float scrollRatio = dragDelta / scrollData->scrollContainerDimensions.height;
                            float newScrollY = initialScrollPosition.y - (scrollRatio * scrollableHeight);
                                 
                            newScrollY = CLAY__MIN(0, CLAY__MAX(newScrollY, -scrollableHeight));
                            
                            *scrollData->scrollPosition = (Clay_Vector2){
                                scrollData->scrollPosition->x,
                                newScrollY
                            };
                        } else if (isHorizontalScrollThumbDragging) {
                            float scrollableWidth = scrollData->contentDimensions.width - scrollData->scrollContainerDimensions.width;
                            float dragDelta = (event.motion.x / globalScalingFactor) - (scrollDragStartX / globalScalingFactor);
                            float scrollRatio = dragDelta / scrollData->scrollContainerDimensions.width;
                            float newScrollX = initialScrollPosition.x - (scrollRatio * scrollableWidth);
                            
                            newScrollX = CLAY__MIN(0, CLAY__MAX(newScrollX, -scrollableWidth));
                            
                            *scrollData->scrollPosition = (Clay_Vector2){
                                newScrollX,
                                scrollData->scrollPosition->y
                            };
                        } else if (isScrollDragging) {
                            float deltaY = (scrollDragStartY - event.motion.y) / globalScalingFactor;
                            float deltaX = (scrollDragStartX - event.motion.x) / globalScalingFactor;
                            
                            float newScrollY = initialScrollPosition.y;
                            float newScrollX = initialScrollPosition.x;
                               
                            if (scrollData->config.vertical) {
                                float scrollableHeight = scrollData->contentDimensions.height - scrollData->scrollContainerDimensions.height;
                                newScrollY = initialScrollPosition.y - deltaY;
                                newScrollY = CLAY__MIN(0, CLAY__MAX(newScrollY, -scrollableHeight));
                            }
                            
                            if (scrollData->config.horizontal) {
                                float scrollableWidth = scrollData->contentDimensions.width - scrollData->scrollContainerDimensions.width;
                                newScrollX = initialScrollPosition.x - deltaX;
                                newScrollX = CLAY__MIN(0, CLAY__MAX(newScrollX, -scrollableWidth));
                            }
                            
                            *scrollData->scrollPosition = (Clay_Vector2){newScrollX, newScrollY};
                            
                            scrollDragStartY = event.motion.y;
                            scrollDragStartX = event.motion.x;
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
                    // Find active scroll container
                    Clay_ScrollContainerData* scrollData = NULL;
                    Clay_ElementId scrollContainerId = {0};
                    Clay_LayoutElementHashMapItem* hashMapItem = NULL;

                    for (int i = 0; i < activeScrollCount; i++) {
                        scrollData = &activeScrollContainers[i];
                        scrollContainerId = activeScrollContainerIds[i];
                        hashMapItem = Clay__GetHashMapItem(scrollContainerId.id);
                        if (hashMapItem) {
                            break;
                        }
                    }

                    if (scrollData && hashMapItem) {
                        Clay_BoundingBox containerBox = hashMapItem->boundingBox;
                        float scaledMouseX = event.button.x / globalScalingFactor;
                        float scaledMouseY = event.button.y / globalScalingFactor;

                        // Vertical scrollbar check
                        if (scrollData->config.vertical) {
                            float viewportHeight = containerBox.height;
                            float contentHeight = scrollData->contentDimensions.height;
                            float thumbHeight = (viewportHeight / contentHeight) * viewportHeight;
                            float thumbPosY = (-scrollData->scrollPosition->y / contentHeight) * viewportHeight;

                            if (scaledMouseX >= containerBox.x + containerBox.width - 10 && 
                                scaledMouseX <= containerBox.x + containerBox.width &&
                                scaledMouseY >= containerBox.y + thumbPosY &&
                                scaledMouseY <= containerBox.y + thumbPosY + thumbHeight) {
                                isScrollThumbDragging = true;
                                scrollDragStartY = event.button.y;
                                initialScrollPosition = *scrollData->scrollPosition;
                            }
                        }
                        
                        // Horizontal scrollbar check
                        if (scrollData->config.horizontal) {
                            float viewportWidth = containerBox.width;
                            float contentWidth = scrollData->contentDimensions.width;
                            float thumbWidth = (viewportWidth / contentWidth) * viewportWidth;
                            float thumbPosX = (-scrollData->scrollPosition->x / contentWidth) * viewportWidth;

                            if (scaledMouseY >= containerBox.y + containerBox.height - 10 && 
                                scaledMouseY <= containerBox.y + containerBox.height &&
                                scaledMouseX >= containerBox.x + thumbPosX &&
                                scaledMouseX <= containerBox.x + thumbPosX + thumbWidth) {
                                isHorizontalScrollThumbDragging = true;
                                scrollDragStartX = event.button.x;
                                initialScrollPosition = *scrollData->scrollPosition;
                            }
                        }

                        // General drag check
                        if (scaledMouseX >= containerBox.x && 
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
                break;
            
            case SDL_MOUSEBUTTONUP:
                if (isScrollThumbDragging || isHorizontalScrollThumbDragging || isScrollDragging) {
                    isScrollThumbDragging = false;
                    isHorizontalScrollThumbDragging = false;
                    isScrollDragging = false;
                }
                Clay_SetPointerState(
                    (Clay_Vector2){(float)event.button.x / globalScalingFactor, (float)event.button.y / globalScalingFactor},
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


#endif
