// android/app/src/main/kotlin/xyz/waozi/myquest/MainActivity.kt
package xyz.waozi.myquest

import org.libsdl.app.SDLActivity
import android.os.Bundle
import android.view.Surface
import android.view.SurfaceHolder
import android.view.WindowManager

class MainActivity : SDLActivity() {
    override fun getLibraries(): Array<String> {
        return arrayOf(
            "SDL2",
            "SDL2_image",
            "SDL2_ttf",
            "main"
        )
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        super.onCreate(savedInstanceState)
    }

    override fun getMainSharedObject(): String {
        return "libmain.so"
    }

    override fun getMainFunction(): String {
        return "SDL_main"
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) {
            hideSystemUI()
        }
    }

    private fun hideSystemUI() {
        window.decorView.systemUiVisibility = (
            android.view.View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
            or android.view.View.SYSTEM_UI_FLAG_LAYOUT_STABLE
            or android.view.View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
            or android.view.View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
            or android.view.View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
            or android.view.View.SYSTEM_UI_FLAG_FULLSCREEN
        )
    }

    companion object {
        init {
            System.loadLibrary("SDL2")
            System.loadLibrary("SDL2_image")
            System.loadLibrary("SDL2_ttf")
            System.loadLibrary("main")
        }
    }
}
