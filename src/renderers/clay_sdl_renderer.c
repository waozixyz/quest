// clay_sdl_renderer.c
#include "renderers/clay_sdl_renderer.h"
#include <unistd.h>
#include "styles.h"
#include "utils.h"

// Global cursor variables
static SDL_Cursor* defaultCursor = NULL;
static SDL_Cursor* pointerCursor = NULL;
static SDL_Cursor* currentCursor = NULL;


static float renderScaleFactor = 1.0f;

void Clay_SDL2_SetRenderScale(float scale) {
    renderScaleFactor = scale;
}

// Helper function to scale a rectangle
static SDL_FRect ScaleBoundingBox(Clay_BoundingBox box) {
    return (SDL_FRect) {
        .x = box.x * renderScaleFactor,
        .y = box.y * renderScaleFactor,
        .w = box.width * renderScaleFactor,
        .h = box.height * renderScaleFactor
    };
}

void Clay_SDL2_InitCursors() {
    // Create default arrow cursor
    defaultCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    
    // Create hand pointer cursor
    pointerCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);

    currentCursor = defaultCursor;
    SDL_SetCursor(currentCursor);
}


void Clay_SDL2_CleanupCursors() {
    if (defaultCursor) {
        SDL_FreeCursor(defaultCursor);
        defaultCursor = NULL;
    }
    if (pointerCursor) {
        SDL_FreeCursor(pointerCursor);
        pointerCursor = NULL;
    }
    currentCursor = NULL;
}

static Clay_Dimensions SDL2_MeasureText(Clay_String *text, Clay_TextElementConfig *config)
{    
    // Validate font
    if (config->fontId >= 32) {
        fprintf(stderr, "ERROR: Invalid fontId: %d\n", config->fontId);
        return (Clay_Dimensions){0, 0};
    }

    TTF_Font *font = SDL2_fonts[config->fontId].font;
    if (!font) {
        fprintf(stderr, "ERROR: Font is NULL for fontId: %d\n", config->fontId);
        return (Clay_Dimensions){0, 0};
    }

    // Additional font validation
    if (TTF_FontHeight(font) <= 0) {
        fprintf(stderr, "ERROR: Invalid font height for fontId: %d\n", config->fontId);
        return (Clay_Dimensions){0, 0};
    }

    if (!text->chars || text->length == 0) {
        // Return a default size for empty text
        return (Clay_Dimensions){0, (float)TTF_FontHeight(font)};
    }

    char *chars = (char *)calloc(text->length + 1, 1);
    if (!chars) {
        fprintf(stderr, "ERROR: Memory allocation failed for text\n");
        return (Clay_Dimensions){0, 0};
    }
    memcpy(chars, text->chars, text->length);
    chars[text->length] = '\0';  // Ensure null-termination

    int width = 0;
    int height = 0;
    int result = TTF_SizeUTF8(font, chars, &width, &height);
    
    if (result < 0) {
        fprintf(stderr, "TTF_SizeUTF8 failed for fontId %d: %s\n", 
                config->fontId, TTF_GetError());
        fprintf(stderr, "Problematic text: '%s'\n", chars);
        free(chars);
        return (Clay_Dimensions){0, 0};
    }

    free(chars);
    return (Clay_Dimensions) {
        .width = (float)width,
        .height = (float)height,
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

SDL_Cursor* Clay_SDL2_GetCurrentCursor() {
    return currentCursor;
}
static void RenderScrollbar(
    SDL_Renderer* renderer,
    Clay_BoundingBox boundingBox,
    bool isVertical,
    int mouseX,
    int mouseY,
    Clay_ScrollElementConfig *config,
    Clay_ElementId elementId
) {
    const float scrollbar_size = 10 * renderScaleFactor;  // Scale scrollbar size
    
    // Scale mouse coordinates for hit testing
    float scaledMouseX = mouseX;  // Already scaled in main render function
    float scaledMouseY = mouseY;  // Already scaled in main render function
    
    // Get scroll data for proper thumb positioning
    Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(elementId);
    if (!scrollData.found) return;

    SDL_FRect scaledBox = ScaleBoundingBox(boundingBox);

    // Calculate scroll ratio based on content vs viewport size
    float viewportSize = isVertical ? scaledBox.h : scaledBox.w;
    float contentSize = isVertical ? scrollData.contentDimensions.height * renderScaleFactor 
                                  : scrollData.contentDimensions.width * renderScaleFactor;
    float scrollPosition = isVertical ? scrollData.scrollPosition->y : scrollData.scrollPosition->x;
    
    // Calculate thumb size and position
    float thumbSize = (viewportSize / contentSize) * viewportSize;
    float thumbPosition = (-scrollPosition / contentSize) * viewportSize * renderScaleFactor;

    // Render background
    SDL_FRect scrollbar_bg = {
        .x = isVertical ? scaledBox.x + scaledBox.w - scrollbar_size : scaledBox.x,
        .y = isVertical ? scaledBox.y : scaledBox.y + scaledBox.h - scrollbar_size,
        .w = isVertical ? scrollbar_size : scaledBox.w,
        .h = isVertical ? scaledBox.h : scrollbar_size
    };
    
    SDL_SetRenderDrawColor(renderer, COLOR_SECONDARY.r, COLOR_SECONDARY.g, COLOR_SECONDARY.b, 200);
    SDL_RenderFillRectF(renderer, &scrollbar_bg);

    // Render thumb
    SDL_FRect scroll_thumb = {
        .x = isVertical ? scrollbar_bg.x : scrollbar_bg.x + thumbPosition,
        .y = isVertical ? scrollbar_bg.y + thumbPosition : scrollbar_bg.y,
        .w = isVertical ? scrollbar_size : thumbSize,
        .h = isVertical ? thumbSize : scrollbar_size
    };

    bool isHovered = scaledMouseX >= scroll_thumb.x && scaledMouseX <= scroll_thumb.x + scroll_thumb.w &&
                    scaledMouseY >= scroll_thumb.y && scaledMouseY <= scroll_thumb.y + scroll_thumb.h;
    
    SDL_SetRenderDrawColor(renderer,
        isHovered ? COLOR_PRIMARY_HOVER.r : COLOR_PRIMARY.r,
        isHovered ? COLOR_PRIMARY_HOVER.g : COLOR_PRIMARY.g,
        isHovered ? COLOR_PRIMARY_HOVER.b : COLOR_PRIMARY.b,
        255
    );
    SDL_RenderFillRectF(renderer, &scroll_thumb);
}


SDL_Rect currentClippingRectangle;

void Clay_SDL2_Render(SDL_Renderer *renderer, Clay_RenderCommandArray renderCommands) {
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
                            
                if (config->cursorPointer == true && 
                    mouseX >= boundingBox.x && mouseX <= boundingBox.x + boundingBox.width &&
                    mouseY >= boundingBox.y && mouseY <= boundingBox.y + boundingBox.height) {
                    hasPointerElement = true;
                }

                Clay_Color color = config->color;
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); 
                SDL_RenderFillRectF(renderer, &scaledBox);
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_TEXT: {
                Clay_TextElementConfig *config = renderCommand->config.textElementConfig;
                Clay_String text = renderCommand->text;

                char *cloned = malloc(text.length + 1);
                if (!cloned) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                        "Memory allocation failed for text rendering");
                    continue;
                }
                memset(cloned, 0, text.length + 1);
                memcpy(cloned, text.chars, text.length);

                TTF_Font* font = SDL2_fonts[config->fontId].font;
                if (!font) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                        "ERROR: Font is NULL for fontId %u", config->fontId);
                    free(cloned);
                    break;
                }

                SDL_Surface *surface = TTF_RenderUTF8_Blended(font, cloned, (SDL_Color) {
                    .r = (Uint8)config->textColor.r,
                    .g = (Uint8)config->textColor.g,
                    .b = (Uint8)config->textColor.b,
                    .a = (Uint8)config->textColor.a,
                });
                
                if (!surface) {
                    fprintf(stderr, "Error rendering text surface: %s\n", TTF_GetError());
                    free(cloned);
                    continue;
                }

                SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
                
                if (!texture) {
                    fprintf(stderr, "Error creating texture: %s\n", SDL_GetError());
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
                SDL_RenderCopyF(renderer, texture, NULL, &destination);

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

                // Scale border widths
                float scaledLeftWidth = config->left.width * renderScaleFactor;
                float scaledRightWidth = config->right.width * renderScaleFactor;
                float scaledTopWidth = config->top.width * renderScaleFactor;
                float scaledBottomWidth = config->bottom.width * renderScaleFactor;

                // Draw left border
                if (config->left.width > 0) {
                    SDL_SetRenderDrawColor(renderer, 
                        config->left.color.r, 
                        config->left.color.g, 
                        config->left.color.b, 
                        config->left.color.a
                    );
                    SDL_FRect leftBorder = {
                        .x = scaledBox.x,
                        .y = scaledBox.y,
                        .w = scaledLeftWidth,
                        .h = scaledBox.h
                    };
                    SDL_RenderFillRectF(renderer, &leftBorder);
                }

                // Draw right border
                if (config->right.width > 0) {
                    SDL_SetRenderDrawColor(renderer, 
                        config->right.color.r, 
                        config->right.color.g, 
                        config->right.color.b, 
                        config->right.color.a
                    );
                    SDL_FRect rightBorder = {
                        .x = scaledBox.x + scaledBox.w - scaledRightWidth,
                        .y = scaledBox.y,
                        .w = scaledRightWidth,
                        .h = scaledBox.h
                    };
                    SDL_RenderFillRectF(renderer, &rightBorder);
                }

                // Draw top border
                if (config->top.width > 0) {
                    SDL_SetRenderDrawColor(renderer, 
                        config->top.color.r, 
                        config->top.color.g, 
                        config->top.color.b, 
                        config->top.color.a
                    );
                    SDL_FRect topBorder = {
                        .x = scaledBox.x,
                        .y = scaledBox.y,
                        .w = scaledBox.w,
                        .h = scaledTopWidth
                    };
                    SDL_RenderFillRectF(renderer, &topBorder);
                }

                // Draw bottom border
                if (config->bottom.width > 0) {
                    SDL_SetRenderDrawColor(renderer, 
                        config->bottom.color.r, 
                        config->bottom.color.g, 
                        config->bottom.color.b, 
                        config->bottom.color.a
                    );
                    SDL_FRect bottomBorder = {
                        .x = scaledBox.x,
                        .y = scaledBox.y + scaledBox.h - scaledBottomWidth,
                        .w = scaledBox.w,
                        .h = scaledBottomWidth
                    };
                    SDL_RenderFillRectF(renderer, &bottomBorder);
                }
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
                SDL_Rect scaledClip = {
                    .x = scaledBox.x,
                    .y = scaledBox.y,
                    .w = scaledBox.w,
                    .h = scaledBox.h
                };
                SDL_RenderSetClipRect(renderer, &scaledClip);

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