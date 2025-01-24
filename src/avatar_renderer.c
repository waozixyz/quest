#include "avatar_renderer.h"
#include <stdlib.h>

// Helper function to draw a pixelated circle
void DrawPixelatedCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int w = -radius; w <= radius; w++) {
        for (int h = -radius; h <= radius; h++) {
            if (w * w + h * h <= radius * radius) {
                SDL_RenderDrawPoint(renderer, centerX + w, centerY + h);
            }
        }
    }
}

// Helper function to draw a pixelated rectangle
void DrawPixelatedRect(SDL_Renderer* renderer, int x, int y, int width, int height, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int w = 0; w < width; w++) {
        for (int h = 0; h < height; h++) {
            SDL_RenderDrawPoint(renderer, x + w, y + h);
        }
    }
}

// Render the face
void RenderFace(SDL_Renderer* renderer, AvatarBounds bounds) {
    SDL_Color skinColor = {236, 240, 241, 255};
    int headRadius = bounds.width / 8;
    int headCenterX = bounds.x + bounds.width / 2;
    int headCenterY = bounds.y + bounds.height / 3;
    DrawPixelatedCircle(renderer, headCenterX, headCenterY, headRadius, skinColor);
}

// Render the eyes
void RenderEyes(SDL_Renderer* renderer, AvatarBounds bounds) {
    SDL_Color eyeColor = {44, 62, 80, 255};
    int eyeRadius = bounds.width / 32;
    int headCenterX = bounds.x + bounds.width / 2;
    int headCenterY = bounds.y + bounds.height / 3;
    DrawPixelatedCircle(renderer, headCenterX - bounds.width / 16, headCenterY - bounds.height / 16, eyeRadius, eyeColor);
    DrawPixelatedCircle(renderer, headCenterX + bounds.width / 16, headCenterY - bounds.height / 16, eyeRadius, eyeColor);
}

// Render the nose
void RenderNose(SDL_Renderer* renderer, AvatarBounds bounds) {
    SDL_Color noseColor = {200, 150, 100, 255};
    int noseWidth = bounds.width / 32;
    int noseHeight = bounds.height / 32;
    int headCenterX = bounds.x + bounds.width / 2;
    int headCenterY = bounds.y + bounds.height / 3;
    DrawPixelatedRect(renderer, headCenterX - noseWidth / 2, headCenterY + bounds.height / 32, noseWidth, noseHeight, noseColor);
}

// Render the mouth
void RenderMouth(SDL_Renderer* renderer, AvatarBounds bounds) {
    SDL_Color mouthColor = {200, 100, 100, 255};
    int mouthWidth = bounds.width / 16;
    int mouthHeight = bounds.height / 32;
    int headCenterX = bounds.x + bounds.width / 2;
    int headCenterY = bounds.y + bounds.height / 3;
    DrawPixelatedRect(renderer, headCenterX - mouthWidth / 2, headCenterY + bounds.height / 16, mouthWidth, mouthHeight, mouthColor);
}

// Render the ears
void RenderEars(SDL_Renderer* renderer, AvatarBounds bounds) {
    SDL_Color earColor = {236, 240, 241, 255};
    int earRadius = bounds.width / 32;
    int headCenterX = bounds.x + bounds.width / 2;
    int headCenterY = bounds.y + bounds.height / 3;
    DrawPixelatedCircle(renderer, headCenterX - bounds.width / 8, headCenterY, earRadius, earColor);
    DrawPixelatedCircle(renderer, headCenterX + bounds.width / 8, headCenterY, earRadius, earColor);
}

// Render the body
void RenderBody(SDL_Renderer* renderer, AvatarBounds bounds) {
    SDL_Color bodyColor = {236, 240, 241, 255};
    int bodyWidth = bounds.width / 4;
    int bodyHeight = bounds.height / 3;
    int bodyTop = bounds.y + bounds.height / 3 + bounds.width / 8;
    DrawPixelatedRect(renderer, bounds.x + bounds.width / 2 - bodyWidth / 2, bodyTop, bodyWidth, bodyHeight, bodyColor);
}

// Render the arms
void RenderArms(SDL_Renderer* renderer, AvatarBounds bounds) {
    SDL_Color armColor = {236, 240, 241, 255};
    int armLength = bounds.width / 4;
    int armY = bounds.y + bounds.height / 3 + bounds.width / 8 + bounds.height / 6;
    for (int i = 0; i < armLength; i++) {
        SDL_RenderDrawPoint(renderer, bounds.x + bounds.width / 2 - bounds.width / 4 - i, armY);
        SDL_RenderDrawPoint(renderer, bounds.x + bounds.width / 2 + bounds.width / 4 + i, armY);
    }
}

// Render the legs
void RenderLegs(SDL_Renderer* renderer, AvatarBounds bounds) {
    SDL_Color legColor = {236, 240, 241, 255};
    int legWidth = bounds.width / 8;
    int legHeight = bounds.height / 4;
    int legTop = bounds.y + bounds.height / 3 + bounds.width / 8 + bounds.height / 3;
    DrawPixelatedRect(renderer, bounds.x + bounds.width / 2 - legWidth, legTop, legWidth, legHeight, legColor);
    DrawPixelatedRect(renderer, bounds.x + bounds.width / 2, legTop, legWidth, legHeight, legColor);
}

// Main function to render the avatar
void RenderProfileAvatar(void* cmd, intptr_t userData) {
    Clay_RenderCommand* render_cmd = (Clay_RenderCommand*)cmd;
    SDL_Renderer* renderer = GetSDLRenderer();
    if (!renderer) return;

    AvatarBounds bounds = {
        .x = render_cmd->boundingBox.x,
        .y = render_cmd->boundingBox.y,
        .width = render_cmd->boundingBox.width,
        .height = render_cmd->boundingBox.height
    };

    // Draw the avatar parts
    RenderFace(renderer, bounds);
    RenderEyes(renderer, bounds);
    RenderNose(renderer, bounds);
    RenderMouth(renderer, bounds);
    RenderEars(renderer, bounds);
    RenderBody(renderer, bounds);
    RenderArms(renderer, bounds);
    RenderLegs(renderer, bounds);
}
