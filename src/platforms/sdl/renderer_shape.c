#include "platforms/sdl/renderer_internal.h"
#include "SDL2_gfxPrimitives.h"

void DrawQuarterCircle(
    SDL_Renderer* renderer,
    float centerX, 
    float centerY, 
    float radius, 
    float startAngle,
    SDL_Color color
) {
    // Increase segments for smoother curves
    const int NUM_SEGMENTS = 32;  // Increased from 16
    float angleStep = (float)(M_PI / 2.0f) / NUM_SEGMENTS;
    
    // We'll use triangle fan rendering for smoother results
    SDL_Vertex* verts = (SDL_Vertex*)alloca((NUM_SEGMENTS + 2) * sizeof(SDL_Vertex));
    int numVerts = 0;

    // Center vertex with alpha blending for antialiasing
    verts[numVerts].position.x = centerX;
    verts[numVerts].position.y = centerY;
    verts[numVerts].color = color;
    numVerts++;

    // Generate vertices with more precise positioning
    for (int i = 0; i <= NUM_SEGMENTS; i++) {
        float angle = startAngle + (i * angleStep);
        // Use double precision for calculation then convert to float
        double precise_x = centerX + cos(angle) * radius;
        double precise_y = centerY + sin(angle) * radius;
        verts[numVerts].position.x = (float)precise_x;
        verts[numVerts].position.y = (float)precise_y;
        verts[numVerts].color = color;
        numVerts++;
    }

    SDL_RenderGeometry(renderer, NULL, verts, numVerts, NULL, 0);
}

void RenderBorder(
    SDL_Renderer* renderer,
    SDL_FRect rect,
    Clay_Border border,
    Clay_CornerRadius cornerRadius,
    bool isTop,
    bool isBottom,
    bool isLeft,
    bool isRight
) {
    float scaledWidth = border.width * renderScaleFactor;
    SDL_Color color = {
        .r = border.color.r,
        .g = border.color.g,
        .b = border.color.b,
        .a = border.color.a
    };

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // For each edge that's enabled, draw both the straight section and any rounded corners
    if (isTop && border.width > 0) {
        // Top straight section
        SDL_FRect topRect = {
            rect.x + (cornerRadius.topLeft * renderScaleFactor),
            rect.y,
            rect.w - ((cornerRadius.topLeft + cornerRadius.topRight) * renderScaleFactor),
            scaledWidth
        };
        SDL_RenderFillRectF(renderer, &topRect);

        // Top-left corner
        if (cornerRadius.topLeft > 0) {
            DrawQuarterCircle(
                renderer,
                rect.x + (cornerRadius.topLeft * renderScaleFactor),
                rect.y + (cornerRadius.topLeft * renderScaleFactor),
                cornerRadius.topLeft * renderScaleFactor,
                M_PI,
                color
            );
        }

        // Top-right corner
        if (cornerRadius.topRight > 0) {
            DrawQuarterCircle(
                renderer,
                rect.x + rect.w - (cornerRadius.topRight * renderScaleFactor),
                rect.y + (cornerRadius.topRight * renderScaleFactor),
                cornerRadius.topRight * renderScaleFactor,
                -M_PI/2,
                color
            );
        }
    }

    // Similar pattern for bottom, left, and right edges
    if (isBottom && border.width > 0) {
        SDL_FRect bottomRect = {
            rect.x + (cornerRadius.bottomLeft * renderScaleFactor),
            rect.y + rect.h - scaledWidth,
            rect.w - ((cornerRadius.bottomLeft + cornerRadius.bottomRight) * renderScaleFactor),
            scaledWidth
        };
        SDL_RenderFillRectF(renderer, &bottomRect);

        if (cornerRadius.bottomLeft > 0) {
            DrawQuarterCircle(
                renderer,
                rect.x + (cornerRadius.bottomLeft * renderScaleFactor),
                rect.y + rect.h - (cornerRadius.bottomLeft * renderScaleFactor),
                cornerRadius.bottomLeft * renderScaleFactor,
                M_PI/2,
                color
            );
        }

        if (cornerRadius.bottomRight > 0) {
            DrawQuarterCircle(
                renderer,
                rect.x + rect.w - (cornerRadius.bottomRight * renderScaleFactor),
                rect.y + rect.h - (cornerRadius.bottomRight * renderScaleFactor),
                cornerRadius.bottomRight * renderScaleFactor,
                0,
                color
            );
        }
    }

    if (isLeft && border.width > 0) {
        SDL_FRect leftRect = {
            rect.x,
            rect.y + (cornerRadius.topLeft * renderScaleFactor),
            scaledWidth,
            rect.h - ((cornerRadius.topLeft + cornerRadius.bottomLeft) * renderScaleFactor)
        };
        SDL_RenderFillRectF(renderer, &leftRect);
    }

    if (isRight && border.width > 0) {
        SDL_FRect rightRect = {
            rect.x + rect.w - scaledWidth,
            rect.y + (cornerRadius.topRight * renderScaleFactor),
            scaledWidth,
            rect.h - ((cornerRadius.topRight + cornerRadius.bottomRight) * renderScaleFactor)
        };
        SDL_RenderFillRectF(renderer, &rightRect);
    }
}
void RenderRoundedRectangle(
    SDL_Renderer* renderer, 
    SDL_FRect rect,
    Clay_CornerRadius cornerRadius,
    Clay_Color color,
    bool shadowEnabled,
    Clay_Color shadowColor,
    Clay_Vector2 shadowOffset,
    float shadowBlurRadius,
    float shadowSpread
) {
    // Render the shadow if enabled
    if (shadowEnabled) {
        // Calculate the shadow rectangle dimensions
        SDL_FRect shadowRect = {
            .x = rect.x + shadowOffset.x - shadowBlurRadius - shadowSpread,
            .y = rect.y + shadowOffset.y - shadowBlurRadius - shadowSpread,
            .w = rect.w + (shadowBlurRadius + shadowSpread) * 2,
            .h = rect.h + (shadowBlurRadius + shadowSpread) * 2
        };

        // Render the shadow as a rounded rectangle
        roundedBoxRGBA(renderer,
            shadowRect.x,
            shadowRect.y,
            shadowRect.x + shadowRect.w,
            shadowRect.y + shadowRect.h,
            cornerRadius.topLeft * renderScaleFactor, // Use the same corner radius as the rectangle
            shadowColor.r, shadowColor.g, shadowColor.b, shadowColor.a);
    }

    // Render the main rectangle
    roundedBoxRGBA(renderer,
        rect.x,
        rect.y,
        rect.x + rect.w,
        rect.y + rect.h,
        cornerRadius.topLeft * renderScaleFactor,
        color.r, color.g, color.b, color.a);
}

void RenderScrollbarRect(
    SDL_Renderer* renderer, 
    SDL_FRect rect,
    Clay_Color color
) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRectF(renderer, &rect);
}
void RenderScrollbar(
    SDL_Renderer* renderer,
    Clay_BoundingBox boundingBox,
    bool isVertical,
    int mouseX,
    int mouseY,
    Clay_ScrollElementConfig *config,
    Clay_ElementId elementId
) {
    Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(elementId);
    if (!scrollData.found) return;

    float viewportSize = isVertical ? boundingBox.height : boundingBox.width;
    float contentSize = isVertical ? scrollData.contentDimensions.height : scrollData.contentDimensions.width;

    if (contentSize <= viewportSize) {
        return;
    }

    SDL_FRect scaledBox = ScaleBoundingBox(boundingBox);
    
    const float scrollbar_size = 10 * renderScaleFactor;

    // Calculate thumb size and position
    float scrollRatio = viewportSize / contentSize;
    float thumbSize = SDL_max(scrollRatio * viewportSize, scrollbar_size * 2) * renderScaleFactor;
    
    float maxScrollContent = contentSize - viewportSize;
    float scrollProgress = isVertical ? 
        (-scrollData.scrollPosition->y / maxScrollContent) :
        (-scrollData.scrollPosition->x / maxScrollContent);

    float maxTrackSize = (isVertical ? scaledBox.h : scaledBox.w) - thumbSize;
    float thumbPosition = scrollProgress * maxTrackSize;
    thumbPosition = SDL_clamp(thumbPosition, 0, maxTrackSize);

    // Create track rect - position differently for horizontal vs vertical
    SDL_FRect track = {
        .x = isVertical ? (scaledBox.x + scaledBox.w - scrollbar_size) : scaledBox.x,
        .y = isVertical ? scaledBox.y : (scaledBox.y + scaledBox.h - scrollbar_size),
        .w = isVertical ? scrollbar_size : scaledBox.w,
        .h = isVertical ? scaledBox.h : scrollbar_size
    };

    // Create thumb rect - position differently for horizontal vs vertical
    SDL_FRect thumb = {
        .x = isVertical ? track.x : (track.x + thumbPosition),
        .y = isVertical ? (track.y + thumbPosition) : track.y,
        .w = isVertical ? scrollbar_size : thumbSize,
        .h = isVertical ? thumbSize : scrollbar_size
    };

    // Hit testing
    float scaledMouseX = mouseX * renderScaleFactor;
    float scaledMouseY = mouseY * renderScaleFactor;
    bool isHovered = 
        scaledMouseX >= thumb.x && scaledMouseX <= thumb.x + thumb.w &&
        scaledMouseY >= thumb.y && scaledMouseY <= thumb.y + thumb.h;

    // Render track
    Clay_Color bgColor = {
        .r = COLOR_PANEL.r,
        .g = COLOR_SECONDARY.g,
        .b = COLOR_SECONDARY.b,
        .a = (Uint8)(200 * scrollbarOpacity)
    };
    RenderScrollbarRect(renderer, track, bgColor);

    // Render thumb
    Clay_Color thumbColor = {
        .r = isHovered ? COLOR_PRIMARY_HOVER.r : COLOR_PRIMARY.r,
        .g = isHovered ? COLOR_PRIMARY_HOVER.g : COLOR_PRIMARY.g,
        .b = isHovered ? COLOR_PRIMARY_HOVER.b : COLOR_PRIMARY.b,
        .a = (Uint8)(255 * scrollbarOpacity)
    };
    RenderScrollbarRect(renderer, thumb, thumbColor);
}