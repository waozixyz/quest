
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
