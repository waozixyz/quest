// clay_sdl_renderer.c
#include "clay_sdl_renderer.h"

typedef struct {
    uint32_t fontId;
    TTF_Font *font;
} SDL2_Font;

static SDL2_Font SDL2_fonts[32];
static Clay_Dimensions SDL2_MeasureText(Clay_String *text, Clay_TextElementConfig *config)
{    
    TTF_Font *font = SDL2_fonts[config->fontId].font;
    if (!font) {
        printf("ERROR: Font is NULL for fontId: %d\n", config->fontId);
        return (Clay_Dimensions){0, 0};
    }

    if (!text->chars) {
        printf("ERROR: text->chars is NULL\n");
        return (Clay_Dimensions){0, 0};
    }

    char *chars = (char *)calloc(text->length + 1, 1);
    memcpy(chars, text->chars, text->length);

    int width = 0;
    int height = 0;
    if (TTF_SizeUTF8(font, chars, &width, &height) < 0) {
        printf("TTF_SizeUTF8 failed: %s\n", TTF_GetError());
        free(chars);
        return (Clay_Dimensions){0, 0};
    }

    free(chars);
    return (Clay_Dimensions) {
            .width = (float)width,
            .height = (float)height,
    };
}

void Clay_SDL2_InitRenderer(void) {
    Clay_SetMeasureTextFunction(SDL2_MeasureText);
}

bool Clay_SDL2_LoadFont(uint32_t fontId, const char* fontPath, int fontSize) {
    printf("Loading font: id=%d, path=%s, size=%d\n", fontId, fontPath, fontSize);
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
    printf("Successfully loaded font id=%d\n", fontId);
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
    for (uint32_t i = 0; i < renderCommands.length; i++)
    {
        Clay_RenderCommand *renderCommand = Clay_RenderCommandArray_Get(&renderCommands, i);
        Clay_BoundingBox boundingBox = renderCommand->boundingBox;
        switch (renderCommand->commandType)
        {
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                Clay_RectangleElementConfig *config = renderCommand->config.rectangleElementConfig;
                Clay_Color color = config->color;
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
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
                Clay_String text = renderCommand->text;
                char *cloned = (char *)calloc(text.length + 1, 1);
                memcpy(cloned, text.chars, text.length);
                TTF_Font* font = SDL2_fonts[config->fontId].font;
                SDL_Surface *surface = TTF_RenderUTF8_Blended(font, cloned, (SDL_Color) {
                        .r = (Uint8)config->textColor.r,
                        .g = (Uint8)config->textColor.g,
                        .b = (Uint8)config->textColor.b,
                        .a = (Uint8)config->textColor.a,
                });
                SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

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

