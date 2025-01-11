#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifndef __EMSCRIPTEN__

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
Java_xyz_waozi_myquest_MainActivity_nativeSetAssetManager(
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
bool Clay_SDL2_LoadFontRW(uint32_t fontId, SDL_RWops* rw, int fontSize);
void cleanup_fonts();


// utils.h
float ScaleUI(float value);
int ScaleUIToInt(float value);
SDL_FRect ScaleRect(SDL_Rect rect);
SDL_FRect ScaleRectF(SDL_FRect rect);

#endif
#endif // UTILS_H
