#include "app.h"
#include "config.h"

#define CLAY_IMPLEMENTATION

#include "clay_extensions.h"
#include "clay.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

CLAY_WASM_EXPORT("UpdateDrawFrame") Clay_RenderCommandArray UpdateDrawFrame(
    float width, 
    float height, 
    float mouseWheelX, 
    float mouseWheelY, 
    float mousePositionX, 
    float mousePositionY, 
    bool isTouchDown, 
    bool isMouseDown, 
    bool arrowKeyDownPressedThisFrame, 
    bool arrowKeyUpPressedThisFrame, 
    bool dKeyPressedThisFrame, 
    float deltaTime
) {
    windowWidth = width;
    windowHeight = height;
    Clay_SetLayoutDimensions((Clay_Dimensions) { width, height });
    Clay_SetPointerState((Clay_Vector2) {mousePositionX, mousePositionY}, isMouseDown || isTouchDown);
    return CreateLayout();
}

int main() {
    return 0;
}

#else

#include <SDL.h>
#include <SDL_ttf.h>  

#ifdef CLAY_MOBILE
int SDL_main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);
    

    if (TTF_Init() < 0) {
        fprintf(stderr, "Error: could not initialize TTF: %s\n", TTF_GetError());
        return 1;
    }

    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);
    
    float ddpi, hdpi, vdpi;
    if (SDL_GetDisplayDPI(0, &ddpi, &hdpi, &vdpi) != 0) {
        ddpi = hdpi = vdpi = 160.0f;
    }
    
    globalScalingFactor = ddpi / 160.0f;
    windowWidth = displayMode.w / globalScalingFactor;
    windowHeight = displayMode.h / globalScalingFactor;
    
    SDL_Window* window = SDL_CreateWindow(
        "myQuest", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        displayMode.w,
        displayMode.h,
        SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN
    );
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    
    if (!window || !renderer) {
        return -1;
    }
    
    RunGameLoop(window, renderer);
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}

#else
int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() < 0) {
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "myQuest",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        windowWidth,
        windowHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!window || !renderer) {
        return 1;
    }

    RunGameLoop(window, renderer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
#endif
#endif