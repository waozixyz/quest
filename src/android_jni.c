#if defined(CLAY_MOBILE)
#include <jni.h>
#include <SDL.h>
#include "utils.h"

JNIEXPORT void JNICALL 
Java_io_naox_quest_MainActivity_nativeSetAssetManager(
    JNIEnv* env, jclass clazz, jobject asset_manager) {
    
    SDL_Log("Native: nativeSetAssetManager called");

    if (!asset_manager) {
        SDL_Log("Native: Asset manager is NULL!");
        return;
    }

    AAssetManager* mgr = AAssetManager_fromJava(env, asset_manager);
    
    if (!mgr) {
        SDL_Log("Native: AAssetManager_fromJava returned NULL!");
        return;
    }

    pthread_mutex_lock(&g_android_state.mutex);
    g_android_state.assetManager = mgr;
    g_android_state.initialized = true;
    pthread_mutex_unlock(&g_android_state.mutex);

    SDL_Log("Native: Asset manager set successfully, pointer: %p", (void*)mgr);
}
#endif