#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "config.h"

#if defined(CLAY_MOBILE)
#include <pthread.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <jni.h>

typedef struct {
    SDL_Window* window;
    AAssetManager* assetManager; 
    pthread_mutex_t mutex;
    bool initialized;
} AndroidState;

extern AndroidState g_android_state;

// JNI method declaration
extern JNIEXPORT void JNICALL 
Java_io_naox_quest_MainActivity_nativeSetAssetManager(
    JNIEnv* env, jclass clazz, jobject asset_manager);
#endif

typedef struct {
    uint32_t fontId;
    TTF_Font *font;
} SDL2_Font;

extern SDL2_Font SDL2_fonts[32];

const char* get_asset_path(const char* filename);
const char* get_font_path(const char* filename);
const char* get_image_path(const char* filename);
bool load_font(uint32_t font_id, const char* filename, int size);
SDL_Surface* load_image(const char* filename);
void cleanup_fonts();
float ScaleUI(float value);
int ScaleUIToInt(float value);
SDL_FRect ScaleRect(SDL_Rect rect);
SDL_FRect ScaleRectF(SDL_FRect rect);

SDL_Renderer* GetSDLRenderer(void);
void SetSDLRenderer(SDL_Renderer* renderer);


size_t strlcpy(char *dst, const char *src, size_t dstsize);
#endif // UTILS_H