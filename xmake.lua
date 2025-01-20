-- Project config
set_project("MyQuest")
set_version("1.0")
set_languages("c99")
add_rules("mode.debug", "mode.release")

add_includedirs("include")
    
-- Android platform configuration
if is_plat("android") then
    add_defines("CLAY_MOBILE")
    add_includedirs("vendor/cJSON", 
                   "vendor/SDL/include",
                   "vendor/SDL_image/include", 
                   "vendor/SDL_ttf",
                   "vendor/SDL2_gfx")
    
    add_files("vendor/cJSON/cJSON.c")
    
    on_load(function (target)
        local arch = target:arch()
        local jniLibs_path = path.absolute(format("android/app/src/main/jniLibs/%s", arch))
        
        -- Add library search paths explicitly
        target:add("linkdirs", jniLibs_path)
        target:add("linkdirs", format("vendor/SDL2_gfx/build-android-%s", arch))
        
        -- Add libraries in exact order
        target:add("links", "SDL2")
        target:add("links", "SDL2_gfx")
        target:add("links", "SDL2_image")
        target:add("links", "SDL2_ttf")
        target:add("links", "android")
        target:add("links", "log")
        target:add("links", "m")
        
        -- Add rpath for runtime library search
        target:add("rpathdirs", jniLibs_path)
        
        -- Force shared library linking
        target:add("shflags", "-shared", {force = true})
    end)
end

-- WASM platform configuration
if is_plat("wasm") then
    add_defines("CLAY_WASM")
    add_includedirs("vendor/clay")
    add_headerfiles("vendor/clay/clay.h")
    add_files("src/**.c")
    remove_files("src/platforms/sdl/**.c")
    
    add_ldflags("-s WASM=1", 
                "-s USE_PTHREADS=0", 
                "-s ASSERTIONS=1", 
                "-s ALLOW_MEMORY_GROWTH=1",
                "-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']",
                "-s EXPORTED_FUNCTIONS=['_main','_printf']",
                {force = true})
end
if is_plat("linux", "macosx", "windows") then
    add_defines("CLAY_DESKTOP")
    
    -- For Linux cross-compilation
    if is_plat("linux") then
        set_toolset("cc", "clang")
    end
    
    add_includedirs("vendor/cJSON") 
    add_files("vendor/cJSON/cJSON.c")
    
    -- Direct SDL linking instead of using packages
    add_links("SDL2", "SDL2_image", "SDL2_ttf", "SDL2_gfx")
    add_links("m")
    
    -- Add SDL includes
    add_includedirs("/usr/include/SDL2")
end

-- Main target configuration
target("main")
    if is_plat("android") then
        set_kind("shared")
        add_cflags("-Wall", "-Werror", "-O2", "-fPIC")
        set_targetdir(path.join("android/app/src/main/jniLibs", "$(ARCH)"))
        
        after_build(function (target)
            local arch = target:arch()
            local jniLibs_dir = path.join("android/app/src/main/jniLibs", arch)
            local target_file = target:targetfile()
            
            os.mkdir(jniLibs_dir)
            os.cp(target_file, path.join(jniLibs_dir, "libmain.so"))
            
            os.mkdir("android/app/src/main/assets/fonts")
            os.mkdir("android/app/src/main/assets/images")
            os.cp("fonts/*", "android/app/src/main/assets/fonts")
            os.cp("images/*", "android/app/src/main/assets/images")
        end)
    elseif is_plat("wasm") then
        set_kind("binary")
        set_targetdir("build/clay")
        add_files("src/**.c")
        remove_files("src/platforms/**.c")  -- ONLY for WASM build
        
        after_build(function (target)
            os.cp("index.html", "build/clay")
            os.cp("manifest.json", "build/clay")
            os.cp("fonts", "build/clay")
            os.cp("images", "build/clay")
        end)
    else
        set_kind("binary")
        set_toolset("cc", "clang")

        add_cflags("-Wall", "-Werror", "-O2")
        add_cflags("-Wno-unused-variable")
        add_cflags("-Wno-missing-braces")



        set_targetdir("build/clay")
        
        after_build(function (target)
            os.cp("fonts", "build/clay")
            os.cp("images", "build/clay")
        end)
    end
    
    add_files("src/**.c")
    set_languages("c99")
-- SDL Android build task
task("build_sdl_android")
    set_menu {
        usage = "xmake build_sdl_android",
        description = "Build SDL dependencies for Android",
        options = {
            {nil, "ARCH", "v", nil, "Set the target architecture"}
        }
    }
    
    on_run(function (target, opt)
        import("core.base.option")
        import("core.project.project")
        
        local ndk_path = os.getenv("NDK_PATH") or "/opt/android-ndk"
        local abis = (opt and opt.ARCH) and {opt.ARCH} or {"armeabi-v7a", "arm64-v8a", "x86", "x86_64"}
        local project_dir = os.curdir()
        
        for _, abi in ipairs(abis) do
            print("Building for " .. abi)
            local jniLibs_dir = path.join(project_dir, "android/app/src/main/jniLibs", abi)
            os.mkdir(jniLibs_dir)
            
            -- Build SDL2 first and wait for completion
            do
                print("Building SDL2...")
                local build_dir = path.join(project_dir, "vendor/SDL/build-android-" .. abi)
                os.mkdir(build_dir)
                os.cd(build_dir)
                os.vexec("cmake .. -DCMAKE_TOOLCHAIN_FILE=%s/build/cmake/android.toolchain.cmake -DANDROID_ABI=%s -DANDROID_PLATFORM=android-21 -DBUILD_SHARED_LIBS=ON", ndk_path, abi)
                os.vexec("make -j4")
                os.cp("libSDL2.so", path.join(jniLibs_dir, "libSDL2.so"))
                os.cd(project_dir)
            end
            
            -- Build SDL2_gfx
            do
                print("Building SDL2_gfx...")
                local build_dir = path.join(project_dir, "vendor/SDL2_gfx/build-android-" .. abi)
                os.mkdir(build_dir)
                os.cd(build_dir)
                os.vexec("cmake .. -DCMAKE_TOOLCHAIN_FILE=%s/build/cmake/android.toolchain.cmake -DANDROID_ABI=%s -DANDROID_PLATFORM=android-21 -DSDL2_DIR=../../SDL/build-android-%s -DSDL2_INCLUDE_DIR=../../SDL/include -DSDL2_LIBRARY=../../SDL/build-android-%s/libSDL2.so -DBUILD_STATIC_LIBS=ON", ndk_path, abi, abi, abi)
                os.vexec("make -j4")
                os.cd(project_dir)
            end
            
            -- Build SDL2_image
            do
                print("Building SDL2_image...")
                local build_dir = path.join(project_dir, "vendor/SDL_image/build-android-" .. abi)
                os.mkdir(build_dir)
                os.cd(build_dir)
                os.vexec("cmake .. -DCMAKE_TOOLCHAIN_FILE=%s/build/cmake/android.toolchain.cmake -DANDROID_ABI=%s -DANDROID_PLATFORM=android-21 -DSDL2_DIR=../../SDL/build-android-%s -DSDL2_INCLUDE_DIR=../../SDL/include -DSDL2_LIBRARY=../../SDL/build-android-%s/libSDL2.so -DSDL2IMAGE_AVIF=OFF -DSDL2IMAGE_WEBP=OFF -DSDL2IMAGE_TIFF=OFF", ndk_path, abi, abi, abi)
                os.vexec("make -j4")
                os.cp("libSDL2_image.so", path.join(jniLibs_dir, "libSDL2_image.so"))
                os.cd(project_dir)
            end
            
            -- Build SDL2_ttf
            do
                print("Building SDL2_ttf...")
                local build_dir = path.join(project_dir, "vendor/SDL_ttf/build-android-" .. abi)
                os.mkdir(build_dir)
                os.cd(build_dir)
                os.vexec("cmake .. -DCMAKE_TOOLCHAIN_FILE=%s/build/cmake/android.toolchain.cmake -DANDROID_ABI=%s -DANDROID_PLATFORM=android-21 -DSDL2_DIR=../../SDL/build-android-%s -DSDL2_INCLUDE_DIR=../../SDL/include -DSDL2_LIBRARY=../../SDL/build-android-%s/libSDL2.so -DBUILD_SHARED_LIBS=ON -DSDL2TTF_SAMPLES=OFF", ndk_path, abi, abi, abi)
                os.vexec("make -j4")
                os.cp("libSDL2_ttf.so", path.join(jniLibs_dir, "libSDL2_ttf.so"))
                os.cd(project_dir)
            end
            
            -- Verify library existence
            print("Verifying libraries for " .. abi)
            local required_libs = {
                "libSDL2.so",
                "libSDL2_image.so",
                "libSDL2_ttf.so"
            }
            
            for _, lib in ipairs(required_libs) do
                local lib_path = path.join(jniLibs_dir, lib)
                if not os.exists(lib_path) then
                    raise("Missing library: " .. lib_path)
                end
            end
            
            -- Check SDL2_gfx static library
            local gfx_lib = path.join(project_dir, "vendor/SDL2_gfx/build-android-" .. abi, "libSDL2_gfx.a")
            if not os.exists(gfx_lib) then
                raise("Missing library: " .. gfx_lib)
            end
            
            os.vexec("ls -l " .. jniLibs_dir)
        end
    end)

task("clean-android")
    set_menu {
        usage = "xmake clean-android",
        description = "Clean Android build files and libraries"
    }
    on_run(function()
        os.rm("vendor/SDL/build-android-*")
        os.rm("vendor/SDL_image/build-android-*")
        os.rm("vendor/SDL_ttf/build-android-*")
        os.rm("vendor/SDL2_gfx/build-android-*")
        os.rm("android/app/build")
        os.rm("android/app/src/main/jniLibs")
    end)