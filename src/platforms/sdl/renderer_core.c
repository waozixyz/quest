#include "platforms/sdl/renderer_internal.h"

// Global variable definitions// Global variable definitions
float renderScaleFactor = 1.0f;
int mouseX = 0;
int mouseY = 0;
float scrollbarOpacity = 0.0f;
float lastMouseMoveTime = 0.0f;
const float SCROLLBAR_FADE_DURATION = 0.6f; 
const float SCROLLBAR_HIDE_DELAY = 0.6f;    

SDL_Rect currentClippingRectangle;

void Clay_SDL2_SetRenderScale(float scale) {
    renderScaleFactor = scale;
}

void* Clay_AllocateAligned(size_t alignment, size_t size) {
    #ifdef CLAY_MOBILE
        // Use Android-compatible aligned allocation
        void* ptr = NULL;
        if (posix_memalign(&ptr, alignment, size) != 0) {
            return NULL;
        }
        return ptr;
    #else
        return aligned_alloc(alignment, size);
    #endif
}

SDL_FRect ScaleBoundingBox(Clay_BoundingBox box) {
    return (SDL_FRect) {
        .x = box.x * renderScaleFactor,
        .y = box.y * renderScaleFactor,
        .w = box.width * renderScaleFactor,
        .h = box.height * renderScaleFactor
    };
}

void Clay_SDL2_InitRenderer(SDL_Renderer *renderer) {
    Clay_SetMeasureTextFunction(SDL2_MeasureText);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    Clay_SDL2_InitCursors();
}

void Clay_SDL2_CleanupRenderer(void) {
    for (int i = 0; i < 32; i++) {
        if (SDL2_fonts[i].font) {
            TTF_CloseFont(SDL2_fonts[i].font);
            SDL2_fonts[i].font = NULL;
        }
    }
    Clay_SDL2_CleanupCursors();
}

void Clay_SDL2_Render(SDL_Renderer *renderer, Clay_RenderCommandArray renderCommands) {
    static float currentTime = 0;
    currentTime = SDL_GetTicks() / 1000.0f;
    
    SDL_GetMouseState(&mouseX, &mouseY);

    // Check for mouse movement
    static int lastMouseX = 0;
    static int lastMouseY = 0;
    
    if (mouseX != lastMouseX || mouseY != lastMouseY) {
        lastMouseMoveTime = currentTime;
        lastMouseX = mouseX;
        lastMouseY = mouseY;
    }

    // Update scrollbar opacity
    float timeSinceLastMove = currentTime - lastMouseMoveTime;
    
    if (timeSinceLastMove < SCROLLBAR_HIDE_DELAY) {
        // Fade in
        scrollbarOpacity = SDL_min(scrollbarOpacity + (1.0f / SCROLLBAR_FADE_DURATION) * (1.0f/60.0f), 1.0f);
    } else {
        // Fade out
        scrollbarOpacity = SDL_max(scrollbarOpacity - (1.0f / SCROLLBAR_FADE_DURATION) * (1.0f/60.0f), 0.0f);
    }

    if (!renderer) {
        fprintf(stderr, "Error: Renderer is NULL\n");
        return;
    }

    bool hasPointerElement = false;

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    // Scale mouse coordinates for hit testing
    mouseX /= renderScaleFactor;
    mouseY /= renderScaleFactor;

    for (uint32_t i = 0; i < renderCommands.length; i++) {
        Clay_RenderCommand *renderCommand = Clay_RenderCommandArray_Get(&renderCommands, i);
        
        if (!renderCommand) {
            fprintf(stderr, "Error: Render command is NULL at index %u\n", i);
            continue;
        }

        Clay_BoundingBox boundingBox = renderCommand->boundingBox;
        SDL_FRect scaledBox = ScaleBoundingBox(boundingBox);

        switch (renderCommand->commandType) {
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                Clay_RectangleElementConfig *config = renderCommand->config.rectangleElementConfig;
                if (!config) {
                    fprintf(stderr, "Error: Rectangle config is NULL\n");
                    continue;
                }
                            
                if (config->cursorPointer && 
                    mouseX >= boundingBox.x && mouseX <= boundingBox.x + boundingBox.width &&
                    mouseY >= boundingBox.y && mouseY <= boundingBox.y + boundingBox.height) {
                    hasPointerElement = true;
                }

                RenderRoundedRectangle(renderer, scaledBox, config->cornerRadius, config->color);
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_TEXT: {
                Clay_TextElementConfig *config = renderCommand->config.textElementConfig;
                Clay_String text = renderCommand->text;

                if (!text.chars || text.length == 0) {
                    continue;
                }

                // Use aligned allocation
                size_t bufferSize = text.length + 1;
                char* cloned = (char*)Clay_AllocateAligned(8, bufferSize);
                
                if (!cloned) {
                    SDL_LogError(SDL_LOG_CATEGORY_ERROR, 
                        "Memory allocation failed for text rendering (size: %zu)", 
                        bufferSize);
                    continue;
                }

                memset(cloned, 0, bufferSize);
                memcpy(cloned, text.chars, text.length);

                TTF_Font* font = SDL2_fonts[config->fontId].font;
                if (!font) {
                    SDL_LogError(SDL_LOG_CATEGORY_ERROR, 
                        "Font is NULL for fontId %u", config->fontId);
                    free(cloned);
                    continue;
                }
                SDL_Color textColor = {
                    .r = (Uint8)config->textColor.r,
                    .g = (Uint8)config->textColor.g,
                    .b = (Uint8)config->textColor.b,
                    .a = (Uint8)config->textColor.a,
                };

                SDL_Surface *surface = TTF_RenderUTF8_Blended(font, cloned, textColor);
                
                if (!surface) {
                    SDL_LogError(SDL_LOG_CATEGORY_ERROR, 
                        "Error rendering text surface: %s", TTF_GetError());
                    free(cloned);
                    continue;
                }

                SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
                
                if (!texture) {
                    SDL_LogError(SDL_LOG_CATEGORY_ERROR, 
                        "Error creating texture: %s", SDL_GetError());
                    SDL_FreeSurface(surface);
                    free(cloned);
                    continue;
                }

                SDL_FRect destination = {
                    .x = scaledBox.x,
                    .y = scaledBox.y,
                    .w = scaledBox.w,
                    .h = scaledBox.h
                };
                
                if (SDL_RenderCopyF(renderer, texture, NULL, &destination) != 0) {
                    SDL_LogError(SDL_LOG_CATEGORY_ERROR, 
                        "Error rendering text texture: %s", SDL_GetError());
                }

                SDL_DestroyTexture(texture);
                SDL_FreeSurface(surface);
                free(cloned);
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_IMAGE: {                
                Clay_ImageElementConfig *imageConfig = renderCommand->config.imageElementConfig;
                SDL_Texture* texture = (SDL_Texture*)imageConfig->imageData;
                SDL_RenderCopyF(renderer, texture, NULL, &scaledBox);
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_BORDER: {
                Clay_BorderElementConfig *config = renderCommand->config.borderElementConfig;
                if (!config) {
                    fprintf(stderr, "Error: Border config is NULL\n");
                    continue;
                }

                // Render each border segment
                RenderBorder(renderer, scaledBox, config->top, config->cornerRadius, true, false, false, false);
                RenderBorder(renderer, scaledBox, config->bottom, config->cornerRadius, false, true, false, false);
                RenderBorder(renderer, scaledBox, config->left, config->cornerRadius, false, false, true, false);
                RenderBorder(renderer, scaledBox, config->right, config->cornerRadius, false, false, false, true);
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
                if (renderCommand->config.scrollElementConfig) {
                    Clay_ScrollElementConfig *config = renderCommand->config.scrollElementConfig;
                    Clay_ElementId elementId = {
                        .id = renderCommand->id,
                        .offset = 0,
                        .baseId = renderCommand->id,
                        .stringId = (Clay_String){ .length = 0, .chars = NULL }
                    };
                    
                    if (config->vertical) {
                        RenderScrollbar(renderer, boundingBox, true, mouseX, mouseY, config, elementId);
                    }
                    if (config->horizontal) {
                        RenderScrollbar(renderer, boundingBox, false, mouseX, mouseY, config, elementId);
                    }
                }

                SDL_Rect scaledClip = {
                    .x = scaledBox.x,
                    .y = scaledBox.y,
                    .w = scaledBox.w,
                    .h = scaledBox.h
                };
                SDL_RenderSetClipRect(renderer, &scaledClip);

                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
                SDL_RenderSetClipRect(renderer, NULL);
                break;
            }

            default: {
                fprintf(stderr, "Error: unhandled render command: %d\n", renderCommand->commandType);
                exit(1);
            }
        }
    }
        
    // Update cursor based on interactive elements
    SDL_Cursor* targetCursor = hasPointerElement ? pointerCursor : defaultCursor;
    if (targetCursor != currentCursor) {
        currentCursor = targetCursor;
        SDL_SetCursor(currentCursor);
    }
}
