#include "renderers/clay_sdl_renderer_internal.h"

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

void RenderRoundedRectangle(
    SDL_Renderer* renderer, 
    SDL_FRect rect,
    Clay_CornerRadius cornerRadius,
    Clay_Color color
) {
    // Scale corner radii
    float topLeftRadius = cornerRadius.topLeft * renderScaleFactor;
    float topRightRadius = cornerRadius.topRight * renderScaleFactor;
    float bottomLeftRadius = cornerRadius.bottomLeft * renderScaleFactor;
    float bottomRightRadius = cornerRadius.bottomRight * renderScaleFactor;

    // Ensure radius doesn't exceed half the rectangle's smaller dimension
    float maxRadius = SDL_min(rect.w / 2, rect.h / 2);
    topLeftRadius = SDL_min(topLeftRadius, maxRadius);
    topRightRadius = SDL_min(topRightRadius, maxRadius);
    bottomLeftRadius = SDL_min(bottomLeftRadius, maxRadius);
    bottomRightRadius = SDL_min(bottomRightRadius, maxRadius);

    // Set up blending for antialiasing
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Color sdlColor = {color.r, color.g, color.b, color.a};

    // Draw center rectangle
    SDL_FRect centerRect = {
        rect.x + topLeftRadius,
        rect.y,
        rect.w - (topLeftRadius + topRightRadius),
        rect.h
    };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRectF(renderer, &centerRect);

    // Draw side rectangles
    SDL_FRect leftRect = {
        rect.x,
        rect.y + topLeftRadius,
        topLeftRadius,
        rect.h - (topLeftRadius + bottomLeftRadius)
    };
    SDL_RenderFillRectF(renderer, &leftRect);

    SDL_FRect rightRect = {
        rect.x + rect.w - topRightRadius,
        rect.y + topRightRadius,
        topRightRadius,
        rect.h - (topRightRadius + bottomRightRadius)
    };
    SDL_RenderFillRectF(renderer, &rightRect);

    // Draw corners with antialiasing
    if (topLeftRadius > 0) {
        DrawQuarterCircle(renderer, rect.x + topLeftRadius, rect.y + topLeftRadius,
                         topLeftRadius, (float)M_PI, sdlColor);
    }
    if (topRightRadius > 0) {
        DrawQuarterCircle(renderer, rect.x + rect.w - topRightRadius, rect.y + topRightRadius,
                         topRightRadius, -(float)M_PI/2, sdlColor);
    }
    if (bottomLeftRadius > 0) {
        DrawQuarterCircle(renderer, rect.x + bottomLeftRadius, rect.y + rect.h - bottomLeftRadius,
                         bottomLeftRadius, (float)M_PI/2, sdlColor);
    }
    if (bottomRightRadius > 0) {
        DrawQuarterCircle(renderer, rect.x + rect.w - bottomRightRadius, rect.y + rect.h - bottomRightRadius,
                         bottomRightRadius, 0, sdlColor);
    }
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
    // Get scroll data for proper thumb positioning 
    Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(elementId);
    if (!scrollData.found) return;

    // Check if content actually exceeds viewport
    float viewportSize = isVertical ? boundingBox.height : boundingBox.width;
    float contentSize = isVertical ? scrollData.contentDimensions.height : scrollData.contentDimensions.width;

    if (contentSize <= viewportSize) {
        return;
    }

    const float scrollbar_size = 10 * renderScaleFactor;
    const float corner_radius = scrollbar_size / 2;

    // Calculate thumb size and position
    float thumbSize = (viewportSize / contentSize) * viewportSize * renderScaleFactor;
    float thumbPosition = (-scrollData.scrollPosition->y / contentSize) * viewportSize * renderScaleFactor;

    // Create scrollbar background rect
    SDL_FRect scrollbar_bg = {
        .x = isVertical ? boundingBox.x + boundingBox.width - scrollbar_size : boundingBox.x,
        .y = isVertical ? boundingBox.y : boundingBox.y + boundingBox.height - scrollbar_size,
        .w = isVertical ? scrollbar_size : boundingBox.width,
        .h = isVertical ? boundingBox.height : scrollbar_size
    };

    // Apply opacity to colors
    Clay_Color bg_color = {
        .r = COLOR_SECONDARY.r,
        .g = COLOR_SECONDARY.g,
        .b = COLOR_SECONDARY.b,
        .a = (Uint8)(200 * scrollbarOpacity)
    };

    Clay_CornerRadius bg_radius = {
        .topLeft = corner_radius,
        .topRight = corner_radius,
        .bottomLeft = corner_radius,
        .bottomRight = corner_radius
    };

    RenderRoundedRectangle(renderer, scrollbar_bg, bg_radius, bg_color);

    // Render thumb with rounded corners
    SDL_FRect scroll_thumb = {
        .x = isVertical ? scrollbar_bg.x : scrollbar_bg.x + thumbPosition,
        .y = isVertical ? scrollbar_bg.y + thumbPosition : scrollbar_bg.y,
        .w = isVertical ? scrollbar_size : thumbSize,
        .h = isVertical ? thumbSize : scrollbar_size
    };

    // Check if mouse is over the thumb
    float scaledMouseX = mouseX;
    float scaledMouseY = mouseY;
    bool thumbHovered = scaledMouseX >= scroll_thumb.x && scaledMouseX <= scroll_thumb.x + scroll_thumb.w &&
                       scaledMouseY >= scroll_thumb.y && scaledMouseY <= scroll_thumb.y + scroll_thumb.h;
    
    Clay_Color thumb_color = {
        .r = thumbHovered ? COLOR_PRIMARY_HOVER.r : COLOR_PRIMARY.r,
        .g = thumbHovered ? COLOR_PRIMARY_HOVER.g : COLOR_PRIMARY.g,
        .b = thumbHovered ? COLOR_PRIMARY_HOVER.b : COLOR_PRIMARY.b,
        .a = (Uint8)(255 * scrollbarOpacity)
    };

    Clay_CornerRadius thumb_radius = {
        .topLeft = corner_radius,
        .topRight = corner_radius,
        .bottomLeft = corner_radius,
        .bottomRight = corner_radius
    };

    RenderRoundedRectangle(renderer, scroll_thumb, thumb_radius, thumb_color);
}