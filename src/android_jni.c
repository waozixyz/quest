#if defined(CLAY_MOBILE)
#include <jni.h>
#include <SDL.h>
#include "utils.h"

// This is the JNI method to set the asset manager
JNIEXPORT void JNICALL 
Java_xyz_waozi_myquest_MainActivity_nativeSetAssetManager(
    JNIEnv* env, jclass clazz, jobject asset_manager) {
    
    pthread_mutex_lock(&g_android_state.mutex);
    g_android_state.assetManager = AAssetManager_fromJava(env, asset_manager);
    pthread_mutex_unlock(&g_android_state.mutex);

    SDL_Log("Native: Asset manager set successfully");
}
#endif