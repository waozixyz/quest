#ifndef AVATAR_RENDERER_H
#define AVATAR_RENDERER_H



#include "clay_extensions.h"
#include "clay.h"
#include "utils.h"
#include <SDL2/SDL.h>
#include <stdint.h>

typedef struct {
    int32_t x, y, width, height;
} AvatarBounds;

void RenderProfileAvatar(void* cmd, intptr_t userData);
void RenderFace(SDL_Renderer* renderer, AvatarBounds bounds);
void RenderEyes(SDL_Renderer* renderer, AvatarBounds bounds);
void RenderNose(SDL_Renderer* renderer, AvatarBounds bounds);
void RenderMouth(SDL_Renderer* renderer, AvatarBounds bounds);
void RenderEars(SDL_Renderer* renderer, AvatarBounds bounds);
void RenderBody(SDL_Renderer* renderer, AvatarBounds bounds);
void RenderArms(SDL_Renderer* renderer, AvatarBounds bounds);
void RenderLegs(SDL_Renderer* renderer, AvatarBounds bounds);

#endif // AVATAR_RENDERER_H
