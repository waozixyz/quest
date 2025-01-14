#include "platforms/sdl/events.h"
#include "app.h"
#include "config.h"
#include "utils.h"
#include <string.h>
#include "clay_extensions.h"
#define CLAY_IMPLEMENTATION

#include "../../vendor/clay/clay.h"
#include "platforms/sdl/renderer.h"



typedef enum {
    TOUCH_STATE_NONE,
    TOUCH_STATE_DOWN,
    TOUCH_STATE_DRAGGING,
} TouchState;

static TouchState currentTouchState = TOUCH_STATE_NONE;
static SDL_FingerID activeTouchId = 0;  // Track the active touch
static Uint32 lastTouchTime = 0;
const Uint32 TOUCH_DEBOUNCE_MS = 200;  // Reduced from 300
const Uint32 POST_CLICK_QUIET_PERIOD_MS = 250;  
static Uint32 lastSuccessfulClickTime = 0;
bool hadMotionBetweenDownAndUp = false;


static const char* const scrollContainerNames[] = {
    "HabitTabs", 
    "CalendarScrollContainer", 
    "HomeScrollContainer"
};
static const int numContainers = sizeof(scrollContainerNames) / sizeof(scrollContainerNames[0]);

static Clay_ScrollContainerData* activeScrollContainer = NULL;
static uint32_t activeScrollContainerId = 0;
static bool isScrollThumbDragging = false;
static bool isHorizontalScrollThumbDragging = false;
static bool isScrollDragging = false;
static float scrollDragStartX = 0;
static float scrollDragStartY = 0;
static Clay_Vector2 initialScrollPosition = {0};
static Clay_Vector2 initialPointerPosition = {0};

typedef struct {
    Clay_BoundingBox boundingBox;
    bool found;
} ElementBoundingData;
ElementBoundingData GetElementBoundingData(Clay_ElementId elementId) {
    ElementBoundingData result = {0};
    
    Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(elementId);
    
    if (scrollData.found) {
        Clay_LayoutElementHashMapItem *hashMapItem = Clay__GetHashMapItem(elementId.id);
        
        if (hashMapItem && hashMapItem->layoutElement) {
            // Use the bounding box from the hash map item
            result.found = true;
            result.boundingBox.x = hashMapItem->boundingBox.x;  // This gives the x position
            result.boundingBox.y = hashMapItem->boundingBox.y;  // This gives the y position
            result.boundingBox.width = scrollData.scrollContainerDimensions.width;
            result.boundingBox.height = scrollData.scrollContainerDimensions.height;
        }
    }
    
    return result;
}
void HandlePointerDragging(float x, float y, Clay_ScrollContainerData* scrollData) {
    if (!scrollData) return;

    if (isScrollThumbDragging) {
        float viewportHeight = scrollData->scrollContainerDimensions.height;
        float contentHeight = scrollData->contentDimensions.height;
        float mouseDelta = (y / globalScalingFactor) - (scrollDragStartY / globalScalingFactor);
        float scrollDelta = -(mouseDelta * (contentHeight / viewportHeight));
        float newScrollY = initialScrollPosition.y + scrollDelta;
        
        // Clamp scroll position
        float scrollableHeight = contentHeight - viewportHeight;
        newScrollY = CLAY__MIN(0, CLAY__MAX(newScrollY, -scrollableHeight));
        
        scrollData->scrollPosition->y = newScrollY;
    } 

    else if (isHorizontalScrollThumbDragging) {
        float viewportWidth = scrollData->scrollContainerDimensions.width;
        float contentWidth = scrollData->contentDimensions.width;
        float mouseDelta = (x / globalScalingFactor) - (scrollDragStartX / globalScalingFactor);
        float scrollDelta = -(mouseDelta * (contentWidth / viewportWidth));
        float newScrollX = initialScrollPosition.x + scrollDelta;
        
        // Clamp the scroll position
        float scrollableWidth = contentWidth - viewportWidth;
        newScrollX = CLAY__MIN(0, CLAY__MAX(newScrollX, -scrollableWidth));
        
        scrollData->scrollPosition->x = newScrollX;
    }
    else if (isScrollDragging) {
        float deltaY = (scrollDragStartY - y) / globalScalingFactor;
        float deltaX = (scrollDragStartX - x) / globalScalingFactor;
        
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
        
        scrollDragStartY = y;
        scrollDragStartX = x;
        initialScrollPosition = *scrollData->scrollPosition;
    }
}

Clay_ScrollContainerData* FindActiveScrollContainer(Clay_Vector2 pointerPosition) {

    printf("Finding Active Scroll Container\n");
    printf("Pointer Position: (%.2f, %.2f)\n", pointerPosition.x, pointerPosition.y);

    // Set the pointer state first
    Clay_SetPointerState(pointerPosition, false);

    for (int i = 0; i < numContainers; i++) {
        Clay_ElementId elementId = Clay_GetElementId((Clay_String){ 
            .length = strlen(scrollContainerNames[i]), 
            .chars = scrollContainerNames[i] 
        });
        
        Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(elementId);
        
        printf("Checking Container: %s\n", scrollContainerNames[i]);
        printf("  Found: %s\n", scrollData.found ? "Yes" : "No");
        
        if (scrollData.found) {
            printf("  Container Dimensions: %.2f x %.2f\n", 
                   scrollData.scrollContainerDimensions.width,
                   scrollData.scrollContainerDimensions.height);
            
            bool isOver = Clay_PointerOver(elementId);
            printf("  Pointer Over: %s\n", isOver ? "Yes" : "No");
            
            if (isOver) {
                Clay_ScrollContainerData* activeScroll = malloc(sizeof(Clay_ScrollContainerData));
                if (activeScroll) {
                    *activeScroll = scrollData;
                    activeScrollContainerId = elementId.id;
                    return activeScroll;
                }
            }
        }
    }
    return NULL;
}
void HandleScrollContainerDragging(float x, float y) {
    static Clay_ScrollContainerData* activeScrollContainer = NULL;
    static Clay_Vector2 initialScrollPosition = {0};
    static float scrollDragStartX = 0;
    static float scrollDragStartY = 0;

    // On first drag, find the active scroll container
    if (!activeScrollContainer) {
        Clay_Vector2 currentPos = {
            x / globalScalingFactor,
            y / globalScalingFactor
        };
        activeScrollContainer = FindActiveScrollContainer(currentPos);
        
        if (activeScrollContainer) {
            scrollDragStartX = x;
            scrollDragStartY = y;
            initialScrollPosition = *activeScrollContainer->scrollPosition;
        }
        return;
    }

    // Perform dragging logic
    if (activeScrollContainer) {
        float deltaY = (scrollDragStartY - y) / globalScalingFactor;
        float deltaX = (scrollDragStartX - x) / globalScalingFactor;
        
        float newScrollY = initialScrollPosition.y;
        float newScrollX = initialScrollPosition.x;
        
        if (activeScrollContainer->config.vertical) {
            float scrollableHeight = activeScrollContainer->contentDimensions.height - 
                                     activeScrollContainer->scrollContainerDimensions.height;
            newScrollY = initialScrollPosition.y - deltaY;
            newScrollY = CLAY__MIN(0, CLAY__MAX(newScrollY, -scrollableHeight));
        }
        
        if (activeScrollContainer->config.horizontal) {
            float scrollableWidth = activeScrollContainer->contentDimensions.width - 
                                    activeScrollContainer->scrollContainerDimensions.width;
            newScrollX = initialScrollPosition.x - deltaX;
            newScrollX = CLAY__MIN(0, CLAY__MAX(newScrollX, -scrollableWidth));
        }
        
        *activeScrollContainer->scrollPosition = (Clay_Vector2){newScrollX, newScrollY};
        
        // Update drag start positions
        scrollDragStartY = y;
        scrollDragStartX = x;
        initialScrollPosition = *activeScrollContainer->scrollPosition;
    }
}

void ResetScrollContainer() {
    isScrollThumbDragging = false;
    isHorizontalScrollThumbDragging = false;
    isScrollDragging = false;
    activeScrollContainerId = 0; 
}

void HandleMouseScrollbarInteraction(
    SDL_MouseButtonEvent* event, 
    Clay_ScrollContainerData* scrollData, 
    Clay_ElementId elementId
) {
    ElementBoundingData boundingData = GetElementBoundingData(elementId);
    if (!boundingData.found) return;
    Clay_BoundingBox containerBox = boundingData.boundingBox;

    float scaledMouseX = event->x / globalScalingFactor;
    float scaledMouseY = event->y / globalScalingFactor;
    
    printf("=== HandleMouseScrollbarInteraction Debug ===\n");
    printf("Container Box:\n");
    printf("  x: %.2f, y: %.2f\n", containerBox.x, containerBox.y);
    printf("  width: %.2f, height: %.2f\n", containerBox.width, containerBox.height);
    printf("Mouse Position (scaled):\n");
    printf("  X: %.2f, Y: %.2f\n", scaledMouseX, scaledMouseY);

    // Vertical Scrollbar Logic
    if (scrollData->config.vertical) {
        // Modify scrollbar area check to be more precise
        bool inScrollbarArea = (scaledMouseX >= containerBox.x + containerBox.width - 15 && 
                              scaledMouseX <= containerBox.x + containerBox.width);
        
        if (inScrollbarArea) {
            float viewportHeight = containerBox.height;
            float contentHeight = scrollData->contentDimensions.height;
            
            // Recalculate thumb dimensions
            float thumbHeight = CLAY__MAX(20.0f, (viewportHeight / contentHeight) * viewportHeight);
            float scrollProgress = -scrollData->scrollPosition->y / (contentHeight - viewportHeight);
            float thumbPosY = containerBox.y + (scrollProgress * (viewportHeight - thumbHeight));
            
            // Check if click is on the thumb
            if (scaledMouseY >= thumbPosY && 
                scaledMouseY <= thumbPosY + thumbHeight) {
                printf("  Clicked on Vertical Thumb\n");
                isScrollThumbDragging = true;
                scrollDragStartY = event->y;
                initialScrollPosition = *scrollData->scrollPosition;
                return;
            }
            
            // Jump scroll logic when clicking outside thumb
            float clickPositionInScrollbar = scaledMouseY - containerBox.y;
            float relativeClickPosition = clickPositionInScrollbar / viewportHeight;
            
            // Determine if we should scroll up or down based on thumb position
            if (relativeClickPosition < scrollProgress) {
                // Click is above current thumb position - scroll up
                scrollData->scrollPosition->y = -(contentHeight - viewportHeight) * relativeClickPosition;
            } else {
                // Click is below current thumb position - scroll down
                scrollData->scrollPosition->y = -(contentHeight - viewportHeight) * relativeClickPosition - thumbHeight;
            }
            
            // Clamp scroll position
            scrollData->scrollPosition->y = CLAY__MIN(0, CLAY__MAX(scrollData->scrollPosition->y, -(contentHeight - viewportHeight)));
            
            printf("Jump Scroll Vertical:\n");
            printf("  Viewport Height: %.2f\n", viewportHeight);
            printf("  Content Height: %.2f\n", contentHeight);
            printf("  New Scroll Position: %.2f\n", scrollData->scrollPosition->y);
            
            return;
        }
    }

    // Horizontal Scrollbar Logic (similar implementation)
    if (scrollData->config.horizontal) {
        bool inScrollbarArea = (scaledMouseY >= containerBox.y + containerBox.height - 15 && 
                              scaledMouseY <= containerBox.y + containerBox.height);
        
        if (inScrollbarArea) {
            float viewportWidth = containerBox.width;
            float contentWidth = scrollData->contentDimensions.width;
            
            // Recalculate thumb dimensions
            float thumbWidth = CLAY__MAX(20.0f, (viewportWidth / contentWidth) * viewportWidth);
            float scrollProgress = -scrollData->scrollPosition->x / (contentWidth - viewportWidth);
            float thumbPosX = containerBox.x + (scrollProgress * (viewportWidth - thumbWidth));
            
            // Check if click is on the thumb
            if (scaledMouseX >= thumbPosX && 
                scaledMouseX <= thumbPosX + thumbWidth) {
                printf("  Clicked on Horizontal Thumb\n");
                isHorizontalScrollThumbDragging = true;
                scrollDragStartX = event->x;
                initialScrollPosition = *scrollData->scrollPosition;
                return;
            }
            
            // Jump scroll logic for horizontal scrolling
            float clickPositionInScrollbar = scaledMouseX - containerBox.x;
            float relativeClickPosition = clickPositionInScrollbar / viewportWidth;
            
            // Determine if we should scroll left or right based on thumb position
            if (relativeClickPosition < scrollProgress) {
                // Click is left of current thumb position - scroll left
                scrollData->scrollPosition->x = -(contentWidth - viewportWidth) * relativeClickPosition;
            } else {
                // Click is right of current thumb position - scroll right
                scrollData->scrollPosition->x = -(contentWidth - viewportWidth) * relativeClickPosition - thumbWidth;
            }
            
            // Clamp scroll position
            scrollData->scrollPosition->x = CLAY__MIN(0, CLAY__MAX(scrollData->scrollPosition->x, -(contentWidth - viewportWidth)));
            
            printf("Jump Scroll Horizontal:\n");
            printf("  Viewport Width: %.2f\n", viewportWidth);
            printf("  Content Width: %.2f\n", contentWidth);
            printf("  New Scroll Position: %.2f\n", scrollData->scrollPosition->x);
            
            return;
        }
    }

    // Default to container drag if no specific thumb interaction
    printf("Defaulting to Container Drag\n");
    isScrollDragging = true;
    scrollDragStartY = event->y;
    scrollDragStartX = event->x;
    initialScrollPosition = *scrollData->scrollPosition;
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
  
                    // Process each scroll container
                    for (int i = 0; i < numContainers; i++) {
                        Clay_ElementId elementId = Clay_GetElementId((Clay_String){ 
                            .length = strlen(scrollContainerNames[i]), 
                            .chars = scrollContainerNames[i] 
                        });
                        
                        Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(elementId);
                        
                        if (scrollData.found) {
                            // Reset horizontal scroll if content now fits
                            if (scrollData.config.horizontal && 
                                scrollData.contentDimensions.width <= windowWidth) {
                                scrollData.scrollPosition->x = 0;
                            }
                            
                            // Reset vertical scroll if content now fits
                            if (scrollData.config.vertical && 
                                scrollData.contentDimensions.height <= windowHeight) {
                                scrollData.scrollPosition->y = 0;
                            }
                        }
                    }

                    Clay_SetLayoutDimensions((Clay_Dimensions){windowWidth, windowHeight});
                }
                break;
            case SDL_MOUSEWHEEL: {
                float scrollMultiplier = 15.0f / globalScalingFactor;
                Clay_Vector2 scrollDelta = {0, 0};

                // Create a current pointer position for finding the active scroll container
                Clay_Vector2 currentPos = {
                    (float)event.wheel.mouseX / globalScalingFactor,
                    (float)event.wheel.mouseY / globalScalingFactor
                };

                // Find the active scroll container using the current pointer position
                Clay_ScrollContainerData* activeScroll = FindActiveScrollContainer(currentPos);

                if (activeScroll) {
                    // Check if we have horizontal scroll input (from touchpad)
                    if (event.wheel.x != 0 && activeScroll->config.horizontal) {
                        // Note: We negate x because positive values should scroll right
                        scrollDelta.x = -event.wheel.x * scrollMultiplier;
                    }
                    // If no horizontal scroll OR horizontal scrolling is disabled,
                    // handle vertical scrolling
                    else if (event.wheel.y != 0) {
                        bool preferHorizontal = (activeScroll->contentDimensions.width > activeScroll->contentDimensions.height) ||
                                            (activeScroll->config.horizontal && !activeScroll->config.vertical);

                        if (preferHorizontal && activeScroll->config.horizontal) {
                            scrollDelta.x = event.wheel.y * scrollMultiplier;
                        } else if (activeScroll->config.vertical) {
                            scrollDelta.y = event.wheel.y * scrollMultiplier;
                        }
                    }

                    Clay_UpdateScrollContainers(true, scrollDelta, delta_time);
                }
            }
            break;

            case SDL_MOUSEBUTTONDOWN:
                initialPointerPosition = (Clay_Vector2){
                    (float)event.button.x / globalScalingFactor,
                    (float)event.button.y / globalScalingFactor
                };
                hadMotionBetweenDownAndUp = false;

                if (event.button.button == SDL_BUTTON_LEFT) {
                    // Add debug print here
                    printf("\nMouse Down:\n");
                    printf("Position: (%.2f, %.2f)\n", initialPointerPosition.x, initialPointerPosition.y);
                    
                    activeScrollContainer = FindActiveScrollContainer(initialPointerPosition);
                    printf("Found container: %s\n", activeScrollContainer ? "Yes" : "No");
                    printf("Container ID: %u\n", activeScrollContainerId);
                    
                    if (activeScrollContainer) {
                        HandleMouseScrollbarInteraction(
                            &event.button, 
                            activeScrollContainer,
                            (Clay_ElementId){.id = activeScrollContainerId}
                        );
                    }
                    
                    // Add debug print here too
                    printf("After interaction:\n");
                    printf("Container Dragging: %s\n", isScrollDragging ? "Yes" : "No");
                }
                break;
                case SDL_MOUSEMOTION:
            {
                Clay_Vector2 currentPos = {
                    (float)event.motion.x / globalScalingFactor,
                    (float)event.motion.y / globalScalingFactor
                };
                
                // Update hover state
                Clay_SetPointerState(currentPos, false);
                
                // Only handle dragging if we have an active scroll container
                if (activeScrollContainer && 
                    (isScrollThumbDragging || isHorizontalScrollThumbDragging || isScrollDragging)) {
                    HandlePointerDragging(event.motion.x, event.motion.y, activeScrollContainer);
                }
            }
            break;
            case SDL_MOUSEBUTTONUP:
                {
                    Clay_Vector2 upPosition = {
                        (float)event.button.x / globalScalingFactor,
                        (float)event.button.y / globalScalingFactor
                    };
                    
                    float dx = upPosition.x - initialPointerPosition.x;
                    float dy = upPosition.y - initialPointerPosition.y;
                    float distanceSquared = dx*dx + dy*dy;
                    
                    if (distanceSquared < 25.0f) {
                        Clay_SetPointerState(upPosition, true);
                    }
                    
                    Clay_SetPointerState(upPosition, false);
                    
                    // Reset all scroll states
                    ResetScrollContainer();
                    activeScrollContainer = NULL;
                }
            break;
            case SDL_FINGERDOWN:
{
                Uint32 currentTime = SDL_GetTicks();
                
                // Check both normal debounce and post-click quiet period
                if (currentTime - lastTouchTime < TOUCH_DEBOUNCE_MS ||
                    currentTime - lastSuccessfulClickTime < POST_CLICK_QUIET_PERIOD_MS) {
                    SDL_Log("Touch ignored - in quiet period (debounce: %u ms, post-click: %u ms)", 
                        currentTime - lastTouchTime,
                        currentTime - lastSuccessfulClickTime);
                    break;
                }
                
                // Only handle the first touch if multiple touches occur
                if (currentTouchState != TOUCH_STATE_NONE) {
                    SDL_Log("Touch ignored - already handling a touch");
                    break;
                }

                // Store this touch's ID as the active one
                activeTouchId = event.tfinger.fingerId;
                currentTouchState = TOUCH_STATE_DOWN;
                
                int actualWidth, actualHeight;
                SDL_GetWindowSize(SDL_GetWindowFromID(event.tfinger.windowID), &actualWidth, &actualHeight);
                
                float screenX = event.tfinger.x * actualWidth;
                float screenY = event.tfinger.y * actualHeight;
                
                SDL_Log("FINGER DOWN: x=%.2f, y=%.2f (screen: %dx%d)", 
                        screenX, screenY, actualWidth, actualHeight);
                
                Clay_Vector2 initialPointerPosition = {
                    screenX / globalScalingFactor,
                    screenY / globalScalingFactor
                };
                
                SDL_Log("Initial position (scaled): x=%.2f, y=%.2f (scale: %.2f)", 
                        initialPointerPosition.x, initialPointerPosition.y, globalScalingFactor);
                
                Clay_SetPointerState(initialPointerPosition, false);
                hadMotionBetweenDownAndUp = false;

                // Find active scroll container using new method
                activeScrollContainer = FindActiveScrollContainer(initialPointerPosition);

                if (activeScrollContainer) {
                    scrollDragStartX = screenX;
                    scrollDragStartY = screenY;
                    initialScrollPosition = *activeScrollContainer->scrollPosition;
                    
                    SDL_Log("Found scroll container with ID: %u", activeScrollContainerId);
                }
            }
            break;

            case SDL_FINGERMOTION:
            {
                // Ignore motion from non-active touches
                if (event.tfinger.fingerId != activeTouchId || currentTouchState == TOUCH_STATE_NONE) {
                    break;
                }

                int actualWidth, actualHeight;
                SDL_GetWindowSize(SDL_GetWindowFromID(event.tfinger.windowID), &actualWidth, &actualHeight);
                
                float screenX = event.tfinger.x * actualWidth;
                float screenY = event.tfinger.y * actualHeight;
                
                Clay_Vector2 currentPos = {
                    screenX / globalScalingFactor,
                    screenY / globalScalingFactor
                };
                
                float dx = screenX - scrollDragStartX;
                float dy = screenY - scrollDragStartY;
                float moveDistance = sqrtf(dx*dx + dy*dy);
                
                SDL_Log("Motion distance: %.2f pixels", moveDistance);
                
                // Start dragging only if we've moved enough
                if (currentTouchState == TOUCH_STATE_DOWN && moveDistance > 40.0f) {
                    currentTouchState = TOUCH_STATE_DRAGGING;
                    isScrollDragging = true;
                    SDL_Log("Scroll drag started (distance threshold reached)");
                }
                
                Clay_SetPointerState(currentPos, false);
                
                // If we're dragging and have an active scroll container, handle dragging
                if (currentTouchState == TOUCH_STATE_DRAGGING) {
                    // If we don't have an active scroll container, try to find one
                    if (!activeScrollContainer) {
                        activeScrollContainer = FindActiveScrollContainer(currentPos);
                    }
                    
                    if (activeScrollContainer) {
                        HandleScrollContainerDragging(screenX, screenY);
                    }
                }
                
                if (moveDistance > 15.0f) {
                    hadMotionBetweenDownAndUp = true;
                }
            }
            break;

            case SDL_FINGERUP:
            {
                // Ignore up events from non-active touches
                if (event.tfinger.fingerId != activeTouchId || currentTouchState == TOUCH_STATE_NONE) {
                    SDL_Log("Ignored non-active touch up event");
                    break;
                }
                
                Uint32 currentTime = SDL_GetTicks();
                lastTouchTime = currentTime;
                
                int actualWidth, actualHeight;
                SDL_GetWindowSize(SDL_GetWindowFromID(event.tfinger.windowID), &actualWidth, &actualHeight);
                
                float screenX = event.tfinger.x * actualWidth;
                float screenY = event.tfinger.y * actualHeight;
                
                Clay_Vector2 upPosition = {
                    screenX / globalScalingFactor,
                    screenY / globalScalingFactor
                };
                
                float dx = (screenX - (initialPointerPosition.x * globalScalingFactor));
                float dy = (screenY - (initialPointerPosition.y * globalScalingFactor));
                float distanceSquared = dx*dx + dy*dy;
                
                // Only process as a tap if we haven't moved much and aren't dragging
                if (distanceSquared < 100.0f && currentTouchState == TOUCH_STATE_DOWN) {
                    SDL_Log("Processing as tap/click");
                    Clay_SetPointerState(upPosition, true);
                    SDL_Delay(25);
                    Clay_SetPointerState(upPosition, false);
                    lastSuccessfulClickTime = SDL_GetTicks();  // Record successful click time
                } else {
                    SDL_Log("Touch ignored - was dragging or moved too much");
                    Clay_SetPointerState(upPosition, false);
                }
                
                // Reset all states
                currentTouchState = TOUCH_STATE_NONE;
                activeTouchId = 0;
                isScrollThumbDragging = false;
                isHorizontalScrollThumbDragging = false;
                isScrollDragging = false;
                hadMotionBetweenDownAndUp = false;
                
                SDL_Log("Touch state reset");
            }
            break;
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



void InitializeSDL(SDL_Window* window, SDL_Renderer* renderer) {
    SDL_Log("Initializing SDL2 renderer...\n");
    Clay_SDL2_InitRenderer(renderer);
    Clay_SDL2_SetRenderScale(globalScalingFactor);  
    SDL_Log("SDL2 renderer initialized\n");

    if (!load_font(FONT_ID_BODY_16, "Quicksand-Semibold.ttf", 16) ||
        !load_font(FONT_ID_TITLE_56, "Calistoga-Regular.ttf", 56) ||
        !load_font(FONT_ID_BODY_24, "Quicksand-Semibold.ttf", 24) ||
        !load_font(FONT_ID_BODY_36, "Quicksand-Semibold.ttf", 36) ||
        !load_font(FONT_ID_TITLE_36, "Calistoga-Regular.ttf", 36) ||
        !load_font(FONT_ID_MONOSPACE_24, "Calistoga-Regular.ttf", 24) ||
        !load_font(FONT_ID_BODY_14, "Quicksand-Semibold.ttf", 14) ||
        !load_font(FONT_ID_BODY_18, "Quicksand-Semibold.ttf", 18)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load one or more fonts");
        TTF_Quit();
        return;
    }
}

void CleanupSDL() {
    Clay_SDL2_CleanupRenderer();
    TTF_Quit();
}
