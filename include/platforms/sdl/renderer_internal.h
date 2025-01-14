#ifndef CLAY_SDL_RENDERER_INTERNAL_H
#define CLAY_SDL_RENDERER_INTERNAL_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include "renderer.h"
#include "../../config.h"
#include "../../utils.h"

// Shared global variables
extern SDL_Cursor* defaultCursor;
extern SDL_Cursor* pointerCursor;
extern SDL_Cursor* currentCursor;
extern float renderScaleFactor;
extern SDL_Rect currentClippingRectangle;


extern int mouseX;
extern int mouseY;

extern float scrollbarOpacity;
extern float lastMouseMoveTime;
extern const float SCROLLBAR_FADE_DURATION;
extern const float SCROLLBAR_HIDE_DELAY;

// Utility functions
SDL_FRect ScaleBoundingBox(Clay_BoundingBox box);

// Shape rendering functions
void RenderBorder(
    SDL_Renderer* renderer,
    SDL_FRect rect,
    Clay_Border border,
    Clay_CornerRadius cornerRadius,
    bool isTop,
    bool isBottom,
    bool isLeft,
    bool isRight
);

void DrawQuarterCircle(
    SDL_Renderer* renderer,
    float centerX, 
    float centerY, 
    float radius, 
    float startAngle,
    SDL_Color color
);

void RenderRoundedRectangle(
    SDL_Renderer* renderer, 
    SDL_FRect rect,
    Clay_CornerRadius cornerRadius,
    Clay_Color color
);

void RenderScrollbar(
    SDL_Renderer* renderer,
    Clay_BoundingBox boundingBox,
    bool isVertical,
    int mouseX,
    int mouseY,
    Clay_ScrollElementConfig *config,
    Clay_ElementId elementId
);

// Text measurement 
Clay_Dimensions SDL2_MeasureText(Clay_String *text, Clay_TextElementConfig *config);


#endif // CLAY_SDL_RENDERER_INTERNAL_H
