#ifndef SDL_EVENTS_H
#define SDL_EVENTS_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>


void InitializeSDL(SDL_Window* window, SDL_Renderer* renderer);
void HandleSDLEvents(bool* running);
void CleanupSDL();


#endif
