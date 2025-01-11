-- Project config
set_project("MyQuest")
set_version("1.0")

-- Add build modes
add_rules("mode.debug", "mode.release")

-- Add includes
add_includedirs("include")

-- Set android configs
-- Set android configs
if is_plat("android") then
    add_defines("CLAY_MOBILE")
    add_includedirs("vendor/cJSON")
    add_includedirs("vendor/SDL/include")
    add_includedirs("vendor/SDL_image/include")
    add_includedirs("vendor/SDL_ttf")
    add_includedirs("vendor/SDL2_gfx")
    
    add_files("vendor/cJSON/cJSON.c")
    
    -- Use specific .so files instead of -l flags
    add_links("android", "log", "m")
    add_linkdirs("android/app/src/main/jniLibs/$(ARCH)")
    add_files("android/app/src/main/jniLibs/$(ARCH)/libSDL2.so")
    add_files("android/app/src/main/jniLibs/$(ARCH)/libSDL2_image.so")
    add_files("android/app/src/main/jniLibs/$(ARCH)/libSDL2_ttf.so")
    add_files("android/app/src/main/jniLibs/$(ARCH)/libSDL2_gfx.so")
    
    -- Add rpath for shared libraries
    add_rpathdirs("android/app/src/main/jniLibs/$(ARCH)")
end
-- Set web configs
if is_plat("wasm") then
    add_defines("CLAY_WASM")
    add_includedirs("vendor/clay")
    add_files("vendor/clay/clay.c")
    
    -- Emscripten flags
    add_ldflags("-s WASM=1", 
                "-s USE_PTHREADS=0", 
                "-s ASSERTIONS=1", 
                "-s ALLOW_MEMORY_GROWTH=1", 
                "-s EXPORTED_RUNTIME_METHODS=[\"ccall\", \"cwrap\"]",
                "-s EXPORTED_FUNCTIONS=[\"_main\", \"_printf\"]",
                {force = true})
end

-- Set desktop configs
if is_plat("linux", "macosx", "windows") then
    add_defines("CLAY_DESKTOP")
    add_includedirs("vendor/cJSON")
    add_files("vendor/cJSON/cJSON.c")
    
    -- Add SDL dependencies
    add_requires("libsdl2", "libsdl_image", "libsdl_ttf", "sdl2_gfx")
    add_packages("libsdl2", "libsdl_image", "libsdl_ttf", "sdl2_gfx")
end

-- Main target
target("myquest")
    -- Set target kind
    if is_plat("android") then
        set_kind("shared")
    else
        set_kind("binary")
    end
    
    -- Add source files
    add_files("src/**.c")
    set_targetdir("build/clay")
    
    -- Post build script to copy assets
    after_build(function (target)
        if is_plat("wasm") then
            os.cp("index.html", "build/clay")
            os.cp("manifest.json", "build/clay")
            os.cp("fonts", "build/clay")
            os.cp("images", "build/clay")
        elseif is_plat("android") then
            os.mkdir("android/app/src/main/assets/fonts")
            os.mkdir("android/app/src/main/assets/images")
            os.cp("fonts/*", "android/app/src/main/assets/fonts")
            os.cp("images/*", "android/app/src/main/assets/images")
        else
            os.cp("fonts", "build/clay")
            os.cp("images", "build/clay")
        end
    end)


    task("build_sdl_android")
    set_menu {
        usage = "xmake build_sdl_android",
        description = "Build SDL dependencies for Android",
        options = {
            {nil, "ARCH", "v", nil, "Set the target architecture"}
        }
    }
    on_run(function (target, opt)
        -- Get NDK path
        local ndk_path = os.getenv("NDK_PATH") or "/opt/android-ndk"
        
        -- Use opt.ARCH if provided, otherwise use all ABIs
        local abis = (opt and opt.ARCH) and {opt.ARCH} or {"armeabi-v7a", "arm64-v8a", "x86", "x86_64"}
        
        for _, abi in ipairs(abis) do
            -- Build SDL2
            os.mkdir("vendor/SDL/build-android-" .. abi)
            os.cd("vendor/SDL/build-android-" .. abi)
            os.exec(string.format(
                "cmake .. -DCMAKE_TOOLCHAIN_FILE=%s/build/cmake/android.toolchain.cmake -DANDROID_ABI=%s -DANDROID_PLATFORM=android-21 -DBUILD_SHARED_LIBS=ON",
                ndk_path,
                abi
            ))
            os.exec("make")
            os.mkdir("android/app/src/main/jniLibs/" .. abi)
            os.cp("libSDL2.so", "android/app/src/main/jniLibs/" .. abi)
            os.cd("../../..")
            
            -- Build SDL2_gfx
            os.mkdir("vendor/SDL2_gfx/build-android-" .. abi)
            os.cd("vendor/SDL2_gfx/build-android-" .. abi)
            os.exec(string.format(
                "cmake .. -DCMAKE_TOOLCHAIN_FILE=%s/build/cmake/android.toolchain.cmake -DANDROID_ABI=%s -DANDROID_PLATFORM=android-21 -DSDL2_DIR=../../SDL/build-android-%s -DSDL2_INCLUDE_DIR=../../SDL/include -DSDL2_LIBRARY=../../SDL/build-android-%s/libSDL2.so",
                ndk_path,
                abi,
                abi,
                abi
            ))
            os.exec("make")
            os.cd("../../..")
            
            -- Build SDL_image
            os.mkdir("vendor/SDL_image/build-android-" .. abi)
            os.cd("vendor/SDL_image/build-android-" .. abi)
            os.exec(string.format(
                "cmake .. -DCMAKE_TOOLCHAIN_FILE=%s/build/cmake/android.toolchain.cmake -DANDROID_ABI=%s -DANDROID_PLATFORM=android-21 -DSDL2_DIR=../../SDL/build-android-%s -DSDL2_INCLUDE_DIR=../../SDL/include -DSDL2_LIBRARY=../../SDL/build-android-%s/libSDL2.so -DSDL2IMAGE_AVIF=OFF -DSDL2IMAGE_WEBP=OFF -DSDL2IMAGE_TIFF=OFF",
                ndk_path,
                abi,
                abi,
                abi
            ))
            os.exec("make")
            os.cp("libSDL2_image.so", "../../../android/app/src/main/jniLibs/" .. abi)
            os.cd("../../..")
            
            -- Build SDL_ttf
            os.mkdir("vendor/SDL_ttf/build-android-" .. abi)
            os.cd("vendor/SDL_ttf/build-android-" .. abi)
            os.exec(string.format(
                "cmake .. -DCMAKE_TOOLCHAIN_FILE=%s/build/cmake/android.toolchain.cmake -DANDROID_ABI=%s -DANDROID_PLATFORM=android-21 -DSDL2_DIR=../../SDL/build-android-%s -DSDL2_INCLUDE_DIR=../../SDL/include -DSDL2_LIBRARY=../../SDL/build-android-%s/libSDL2.so -DBUILD_SHARED_LIBS=ON -DSDL2TTF_SAMPLES=OFF",
                ndk_path,
                abi,
                abi,
                abi
            ))
            os.exec("make")
            os.cp("libSDL2_ttf.so", "../../../android/app/src/main/jniLibs/" .. abi)
            os.cd("../../..")
        end
    end)