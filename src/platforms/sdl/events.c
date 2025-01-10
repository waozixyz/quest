#include "platforms/sdl/events.h"
#include "app.h"
#include "config.h"
#include "utils.h"
#include "clay_extensions.h"
#define CLAY_IMPLEMENTATION

#include "../../vendor/clay/clay.h"
#include "platforms/sdl/renderer.h"

bool hadMotionBetweenDownAndUp = false;
Clay_Vector2 initialPointerPosition = {0};
bool isScrollThumbDragging = false;
bool isHorizontalScrollThumbDragging = false;
bool isScrollDragging = false;
float scrollDragStartX = 0;
float scrollDragStartY = 0;
Clay_Vector2 initialScrollPosition = {0};


void HandlePointerDragging(float x, float y, Clay_ScrollContainerData* scrollData) {
    if (!scrollData) return;

    if (isScrollThumbDragging) {
        float viewportHeight = scrollData->scrollContainerDimensions.height;
        float contentHeight = scrollData->contentDimensions.height;
        float mouseDelta = (y / globalScalingFactor) - (scrollDragStartY / globalScalingFactor);
        float scrollDelta = -(mouseDelta * (contentHeight / viewportHeight));
        float newScrollY = initialScrollPosition.y + scrollDelta;
        
        // Clamp the scroll position
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
            // Modify the window resize event handling
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    windowWidth = event.window.data1 / globalScalingFactor;
                    windowHeight = event.window.data2 / globalScalingFactor;
                                                            
                    // Get ALL scroll containers in the app dynamically
                    Clay_ScrollContainerData scrollContainersToCheck[16] = {0};
                    int scrollContainerCount = 0;

                    // Iterate through ALL layout elements to find scroll containers
                    for (uint32_t i = 0; i < Clay__layoutElements.length && scrollContainerCount < 16; i++) {
                        Clay_LayoutElement *element = Clay_LayoutElementArray_Get(&Clay__layoutElements, i);
                        
                        // Check if this element has scroll configuration
                        for (int32_t configIndex = 0; configIndex < element->elementConfigs.length; configIndex++) {
                            Clay_ElementConfig *config = Clay__ElementConfigArraySlice_Get(&element->elementConfigs, configIndex);
                            
                            if (config->type == CLAY__ELEMENT_CONFIG_TYPE_SCROLL_CONTAINER) {
                                Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData((Clay_ElementId){.id = element->id});
                                
                                if (scrollData.found) {
                                    scrollContainersToCheck[scrollContainerCount++] = scrollData;
                                    break;  // Only add each element once
                                }
                            }
                        }
                    }

                    // Process each found scroll container
                    for (int i = 0; i < scrollContainerCount; i++) {
                        Clay_ScrollContainerData* data = &scrollContainersToCheck[i];
                    
                        // Reset horizontal scroll if content now fits
                        if (data->config.horizontal && 
                            data->contentDimensions.width <= windowWidth) {
                            data->scrollPosition->x = 0;
                        }
                        
                        // Reset vertical scroll if content now fits
                        if (data->config.vertical && 
                            data->contentDimensions.height <= windowHeight) {
                            data->scrollPosition->y = 0;
                        }
                    }

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
                }

                Clay_UpdateScrollContainers(true, scrollDelta, delta_time);
            }
            break;
            case SDL_MOUSEBUTTONDOWN:
                
                initialPointerPosition = (Clay_Vector2){
                    (float)event.button.x / globalScalingFactor,
                    (float)event.button.y / globalScalingFactor
                };
                hadMotionBetweenDownAndUp = false;

                if (event.button.button == SDL_BUTTON_LEFT) {
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

                        // Vertical scrollbar handling
                        if (scrollData->config.vertical) {
                            float viewportHeight = containerBox.height;
                            float contentHeight = scrollData->contentDimensions.height;
                            float thumbHeight = (viewportHeight / contentHeight) * viewportHeight;
                            float thumbPosY = (-scrollData->scrollPosition->y / contentHeight) * viewportHeight;

                            // Check if click is in scrollbar area
                            if (scaledMouseX >= containerBox.x + containerBox.width - 10 && 
                                scaledMouseX <= containerBox.x + containerBox.width) {
                                
                                // Click on thumb - start dragging
                                if (scaledMouseY >= containerBox.y + thumbPosY &&
                                    scaledMouseY <= containerBox.y + thumbPosY + thumbHeight) {
                                    isScrollThumbDragging = true;
                                    scrollDragStartY = event.button.y;
                                    initialScrollPosition = *scrollData->scrollPosition;
                                }
                                // Click on scrollbar (not on thumb) - jump scroll
                                else {
                                    float clickPositionRatio = (scaledMouseY - containerBox.y) / viewportHeight;
                                    float newScrollY = -(clickPositionRatio * contentHeight - viewportHeight/2);
                                    float scrollableHeight = contentHeight - viewportHeight;
                                    newScrollY = CLAY__MIN(0, CLAY__MAX(newScrollY, -scrollableHeight));
                                    scrollData->scrollPosition->y = newScrollY;
                                }
                            }
                        }

                        // Horizontal scrollbar handling
                        if (scrollData->config.horizontal) {
                            float viewportWidth = containerBox.width;
                            float contentWidth = scrollData->contentDimensions.width;
                            float thumbWidth = (viewportWidth / contentWidth) * viewportWidth;
                            float thumbPosX = (-scrollData->scrollPosition->x / contentWidth) * viewportWidth;

                            // Check if click is in scrollbar area
                            if (scaledMouseY >= containerBox.y + containerBox.height - 10 && 
                                scaledMouseY <= containerBox.y + containerBox.height) {
                                
                                // Click on thumb - start dragging
                                if (scaledMouseX >= containerBox.x + thumbPosX &&
                                    scaledMouseX <= containerBox.x + thumbPosX + thumbWidth) {
                                    isHorizontalScrollThumbDragging = true;
                                    scrollDragStartX = event.button.x;
                                    initialScrollPosition = *scrollData->scrollPosition;
                                }
                                // Click on scrollbar (not on thumb) - jump scroll
                                else {
                                    float clickPositionRatio = (scaledMouseX - containerBox.x) / viewportWidth;
                                    float newScrollX = -(clickPositionRatio * contentWidth - viewportWidth/2);
                                    float scrollableWidth = contentWidth - viewportWidth;
                                    newScrollX = CLAY__MIN(0, CLAY__MAX(newScrollX, -scrollableWidth));
                                    scrollData->scrollPosition->x = newScrollX;
                                }
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
            case SDL_MOUSEMOTION:
                {
                    Clay_Vector2 currentPos = {
                        (float)event.motion.x / globalScalingFactor,
                        (float)event.motion.y / globalScalingFactor
                    };
                    
                    // Just update position for hover effects, don't set pressed state
                    Clay_SetPointerState(currentPos, false);
                    if (isScrollThumbDragging || isHorizontalScrollThumbDragging || isScrollDragging) {
                        Clay_ScrollContainerData* scrollData = NULL;
                        for (int i = 0; i < activeScrollCount; i++) {
                            scrollData = &activeScrollContainers[i];
                            break;
                        }
                        HandlePointerDragging(event.motion.x, event.motion.y, scrollData);
                    }
                }
                break;

            
            case SDL_MOUSEBUTTONUP:
                {

                    Clay_Vector2 upPosition = {
                        (float)event.button.x / globalScalingFactor,
                        (float)event.button.y / globalScalingFactor
                    };
                    
                    // Only trigger the click if motion was minimal (less than 5 pixels)
                    float dx = upPosition.x - initialPointerPosition.x;
                    float dy = upPosition.y - initialPointerPosition.y;
                    float distanceSquared = dx*dx + dy*dy;
                    if (distanceSquared < 25.0f) { // 5 pixels squared
                        Clay_SetPointerState(upPosition, true);  // Trigger the click
                    }
                    
                    // Always reset the pointer state
                    Clay_SetPointerState(upPosition, false);
                    
                    // Reset scroll states
                    isScrollThumbDragging = false;
                    isHorizontalScrollThumbDragging = false;
                    isScrollDragging = false;
                }
                break;
            case SDL_FINGERDOWN:
                {
                    int actualWidth, actualHeight;
                    SDL_GetWindowSize(SDL_GetWindowFromID(event.tfinger.windowID), &actualWidth, &actualHeight);
                    
                    float screenX = event.tfinger.x * actualWidth;
                    float screenY = event.tfinger.y * actualHeight;
                    
                    initialPointerPosition = (Clay_Vector2){
                        screenX / globalScalingFactor,
                        screenY / globalScalingFactor
                    };
                    
                    Clay_SetPointerState(initialPointerPosition, false);
                    hadMotionBetweenDownAndUp = false;

                    // Don't immediately start scroll dragging on touch down
                    // Instead, wait for some movement to determine if it's a scroll or tap
                    Clay_ScrollContainerData* scrollData = NULL;
                    for (int i = 0; i < activeScrollCount; i++) {
                        scrollData = &activeScrollContainers[i];
                        break;
                    }

                    if (scrollData) {
                        scrollDragStartX = screenX;
                        scrollDragStartY = screenY;
                        initialScrollPosition = *scrollData->scrollPosition;
                        // Don't set isScrollDragging immediately
                    }
                }
                break;
            case SDL_FINGERMOTION:
                {
                    int actualWidth, actualHeight;
                    SDL_GetWindowSize(SDL_GetWindowFromID(event.tfinger.windowID), &actualWidth, &actualHeight);
                    
                    float screenX = event.tfinger.x * actualWidth;
                    float screenY = event.tfinger.y * actualHeight;
                    
                    Clay_Vector2 currentPos = {
                        screenX / globalScalingFactor,
                        screenY / globalScalingFactor
                    };
                    
                    // Calculate movement distance
                    float dx = screenX - scrollDragStartX;
                    float dy = screenY - scrollDragStartY;
                    float moveDistance = sqrtf(dx*dx + dy*dy);
                    
                    // Increase threshold for scroll start on mobile
                    if (!isScrollDragging && moveDistance > 40.0f) {  // Increased from 10 to 20
                        isScrollDragging = true;
                    }
                    
                    Clay_SetPointerState(currentPos, false);
                    
                    if (isScrollDragging) {
                        Clay_ScrollContainerData* scrollData = NULL;
                        for (int i = 0; i < activeScrollCount; i++) {
                            scrollData = &activeScrollContainers[i];
                            break;
                        }
                        if (scrollData) {
                            HandlePointerDragging(screenX, screenY, scrollData);
                        }
                    }
                    
                    // Increase motion threshold for touch
                    if (moveDistance > 15.0f) {  // Increased from 5 to 15
                        hadMotionBetweenDownAndUp = true;
                    }
                }
                break;

            case SDL_FINGERUP:
                {
                    int actualWidth, actualHeight;
                    SDL_GetWindowSize(SDL_GetWindowFromID(event.tfinger.windowID), &actualWidth, &actualHeight);
                    
                    float screenX = event.tfinger.x * actualWidth;
                    float screenY = event.tfinger.y * actualHeight;
                    
                    Clay_Vector2 upPosition = {
                        screenX / globalScalingFactor,
                        screenY / globalScalingFactor
                    };
                    
                    float dx = upPosition.x - initialPointerPosition.x;
                    float dy = upPosition.y - initialPointerPosition.y;
                    float distanceSquared = dx*dx + dy*dy;
                    
                    // Increase the allowed movement for a touch
                    if (distanceSquared < 100.0f) {  // Increased from 25 to 100 (10 pixels squared)
                        Clay_SetPointerState(upPosition, true);
                        SDL_Delay(1);
                        Clay_SetPointerState(upPosition, false);
                    } else {
                        Clay_SetPointerState(upPosition, false);
                    }
                    
                    isScrollThumbDragging = false;
                    isHorizontalScrollThumbDragging = false;
                    isScrollDragging = false;
                    hadMotionBetweenDownAndUp = false;
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
