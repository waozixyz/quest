package xyz.waozi.myquest

import android.content.res.AssetManager
import android.os.Bundle
import android.util.Log
import android.view.WindowManager
import org.libsdl.app.SDLActivity
import org.libsdl.app.SDL

class MainActivity : SDLActivity() {
   companion object {
       @JvmStatic
       external fun nativeSetAssetManager(assetManager: AssetManager)

       init {
           Log.d("MainActivity", "Starting to load libraries...")
           try {
               System.loadLibrary("SDL2")
               Log.d("MainActivity", "Loaded SDL2 successfully")
               
               System.loadLibrary("SDL2_image")
               Log.d("MainActivity", "Loaded SDL2_image successfully")
               
               System.loadLibrary("SDL2_ttf")
               Log.d("MainActivity", "Loaded SDL2_ttf successfully")
               
               System.loadLibrary("main")
               Log.d("MainActivity", "Loaded main library successfully")
               
               SDL.setupJNI()
               Log.d("MainActivity", "SDL JNI setup completed successfully")
           } catch (e: Exception) {
               Log.e("MainActivity", "Failed to load libraries", e)
               e.printStackTrace()
           }
       }
   }

   override fun getLibraries(): Array<String> = arrayOf(
       "SDL2", "SDL2_image", "SDL2_ttf", "main"
   ).also {
       Log.d("MainActivity", "getLibraries called, returning: ${it.joinToString()}")
   }

   override fun onCreate(savedInstanceState: Bundle?) {
       Log.d("MainActivity", "onCreate starting - initializing SDL")
       try {
           SDL.initialize()
           Log.d("MainActivity", "SDL initialized successfully")
           SDL.setContext(this)
           Log.d("MainActivity", "SDL context set")

           nativeSetAssetManager(assets)
           Log.d("MainActivity", "Asset manager set successfully")
       } catch (e: Exception) {
           Log.e("MainActivity", "Failed to initialize SDL", e)
       }
       
       try {
           super.onCreate(savedInstanceState)
           Log.d("MainActivity", "super.onCreate completed")
       } catch (e: Exception) {
           Log.e("MainActivity", "Error in super.onCreate", e)
       }
       
       window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
       Log.d("MainActivity", "onCreate completed")
   }

   override fun onResume() {
       Log.d("MainActivity", "onResume starting")
       try {
           super.onResume()
           SDL.setContext(this)
           Log.d("MainActivity", "onResume completed successfully")
       } catch (e: Exception) {
           Log.e("MainActivity", "Error in onResume", e)
       }
   }

   override fun onPause() {
       Log.d("MainActivity", "onPause starting")
       try {
           super.onPause()
           Log.d("MainActivity", "onPause completed")
       } catch (e: Exception) {
           Log.e("MainActivity", "Error in onPause", e)
       }
   }

   override fun onDestroy() {
       Log.d("MainActivity", "onDestroy starting")
       try {
           SDL.setContext(null)
           super.onDestroy()
           Log.d("MainActivity", "onDestroy completed")
       } catch (e: Exception) {
           Log.e("MainActivity", "Error in onDestroy", e)
       }
   }

   override fun getMainSharedObject(): String = "libmain.so"
   override fun getMainFunction(): String = "SDL_main"

   override fun onWindowFocusChanged(hasFocus: Boolean) {
       super.onWindowFocusChanged(hasFocus)
       Log.d("MainActivity", "onWindowFocusChanged: hasFocus=$hasFocus")
   }
}