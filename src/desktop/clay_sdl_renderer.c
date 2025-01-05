// clay_sdl_renderer.c
#include "clay_sdl_renderer.h"
#include <unistd.h>


typedef struct {
    uint32_t fontId;
    TTF_Font *font;
} SDL2_Font;

static SDL2_Font SDL2_fonts[32];
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

}

bool Clay_SDL2_LoadFont(uint32_t fontId, const char* fontPath, int fontSize) {
    if (!fontPath || access(fontPath, F_OK) != 0) {
        fprintf(stderr, "Error: Font file does not exist at %s\n", fontPath);
        return false;
    }

    if (fontId >= 32) {
        printf("ERROR: fontId %d is too large\n", fontId);
        return false;
    }
    TTF_Font* font = TTF_OpenFont(fontPath, fontSize);
    if (!font) {
        printf("ERROR: Failed to load font: %s\n", TTF_GetError());
        return false;
    }
    SDL2_fonts[fontId].font = font;
    SDL2_fonts[fontId].fontId = fontId;
    return true;
}

void Clay_SDL2_CleanupRenderer(void) {
    for (int i = 0; i < 32; i++) {
        if (SDL2_fonts[i].font) {
            TTF_CloseFont(SDL2_fonts[i].font);
            SDL2_fonts[i].font = NULL;
        }
    }
}

SDL_Rect currentClippingRectangle;

void Clay_SDL2_Render(SDL_Renderer *renderer, Clay_RenderCommandArray renderCommands)
{
    if (!renderer) {
        fprintf(stderr, "Error: Renderer is NULL\n");
        return;
    }
    for (uint32_t i = 0; i < renderCommands.length; i++)
    {
        Clay_RenderCommand *renderCommand = Clay_RenderCommandArray_Get(&renderCommands, i);
        
        // Add null checks
        if (!renderCommand) {
            fprintf(stderr, "Error: Render command is NULL at index %u\n", i);
            continue;
        }

        Clay_BoundingBox boundingBox = renderCommand->boundingBox;

        switch (renderCommand->commandType)
        {
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                Clay_RectangleElementConfig *config = renderCommand->config.rectangleElementConfig;
                if (!config) {
                    fprintf(stderr, "Error: Rectangle config is NULL\n");
                    continue;
                }
                Clay_Color color = config->color;
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); 
                SDL_FRect rect = (SDL_FRect) {
                        .x = boundingBox.x,
                        .y = boundingBox.y,
                        .w = boundingBox.width,
                        .h = boundingBox.height,
                };
                SDL_RenderFillRectF(renderer, &rect);
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_TEXT: {
                Clay_TextElementConfig *config = renderCommand->config.textElementConfig;
                if (!config) {
                    fprintf(stderr, "Error: Text config is NULL\n");
                    continue;
                }
                Clay_String text = renderCommand->text;
                
                // Add more rigorous text validation
                if (!text.chars || text.length == 0) {
                    fprintf(stderr, "Error: Text is empty or NULL\n");
                    continue;
                }

                char *cloned = (char *)calloc(text.length + 1, 1);
                if (!cloned) {
                    fprintf(stderr, "Error: Memory allocation failed for text\n");
                    continue;
                }
                memcpy(cloned, text.chars, text.length);

                TTF_Font* font = SDL2_fonts[config->fontId].font;
                if (!font) {
                    fprintf(stderr, "Error: Font is NULL for fontId %u\n", config->fontId);
                    free(cloned);
                    continue;
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

                SDL_Rect destination = (SDL_Rect){
                        .x = boundingBox.x,
                        .y = boundingBox.y,
                        .w = boundingBox.width,
                        .h = boundingBox.height,
                };
                SDL_RenderCopy(renderer, texture, NULL, &destination);

                SDL_DestroyTexture(texture);
                SDL_FreeSurface(surface);
                free(cloned);
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_BORDER: {
                Clay_BorderElementConfig *config = renderCommand->config.borderElementConfig;
                if (!config) {
                    fprintf(stderr, "Error: Border config is NULL\n");
                    continue;
                }

                // Set border colors and draw borders
                SDL_SetRenderDrawColor(renderer, 
                    config->left.color.r, 
                    config->left.color.g, 
                    config->left.color.b, 
                    config->left.color.a
                );

                // Draw left border
                if (config->left.width > 0) {
                    SDL_Rect leftBorder = {
                        .x = boundingBox.x,
                        .y = boundingBox.y,
                        .w = config->left.width,
                        .h = boundingBox.height
                    };
                    SDL_RenderFillRect(renderer, &leftBorder);
                }

                // Draw right border
                if (config->right.width > 0) {
                    SDL_SetRenderDrawColor(renderer, 
                        config->right.color.r, 
                        config->right.color.g, 
                        config->right.color.b, 
                        config->right.color.a
                    );
                    SDL_Rect rightBorder = {
                        .x = boundingBox.x + boundingBox.width - config->right.width,
                        .y = boundingBox.y,
                        .w = config->right.width,
                        .h = boundingBox.height
                    };
                    SDL_RenderFillRect(renderer, &rightBorder);
                }

                // Draw top border
                if (config->top.width > 0) {
                    SDL_SetRenderDrawColor(renderer, 
                        config->top.color.r, 
                        config->top.color.g, 
                        config->top.color.b, 
                        config->top.color.a
                    );
                    SDL_Rect topBorder = {
                        .x = boundingBox.x,
                        .y = boundingBox.y,
                        .w = boundingBox.width,
                        .h = config->top.width
                    };
                    SDL_RenderFillRect(renderer, &topBorder);
                }

                // Draw bottom border
                if (config->bottom.width > 0) {
                    SDL_SetRenderDrawColor(renderer, 
                        config->bottom.color.r, 
                        config->bottom.color.g, 
                        config->bottom.color.b, 
                        config->bottom.color.a
                    );
                    SDL_Rect bottomBorder = {
                        .x = boundingBox.x,
                        .y = boundingBox.y + boundingBox.height - config->bottom.width,
                        .w = boundingBox.width,
                        .h = config->bottom.width
                    };
                    SDL_RenderFillRect(renderer, &bottomBorder);
                }
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
                currentClippingRectangle = (SDL_Rect) {
                        .x = boundingBox.x,
                        .y = boundingBox.y,
                        .w = boundingBox.width,
                        .h = boundingBox.height,
                };
                SDL_RenderSetClipRect(renderer, &currentClippingRectangle);
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
}

