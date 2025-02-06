-- Project config
set_project("Quest")
set_version("1.0")
set_languages("c99")
add_rules("mode.debug", "mode.release")

-- Options
option("renderer")
    set_default("sdl2")
    set_values("sdl2", "sdl3", "raylib")
    set_showmenu(true)
    set_description("Choose renderer backend")

-- Main target configuration
target("main")
    -- Set defaults that apply across all platforms
    set_kind("binary")
    set_languages("c99")
    
    -- Include base paths
    add_includedirs("include", "../rocks/include", "../rocks/include/renderer", "../rocks/include/components", "../rocks/clay")
    add_files("src/**.c")
    add_files("../rocks/src/*.c", "../rocks/src/components/*.c")

    -- Add renderer-specific files based on choice
    if get_config("renderer") == "sdl2" or get_config("renderer") == "sdl3" then
        add_files("../rocks/src/renderer/sdl2_renderer.c", "../rocks/src/renderer/sdl2_renderer_utils.c")
        add_defines("ROCKS_USE_SDL2")
    elseif get_config("renderer") == "raylib" then
        add_files("../rocks/src/renderer/raylib_renderer.c")
        add_defines("ROCKS_USE_RAYLIB")
    end

    -- Android platform configuration
    if is_plat("android") then
        set_kind("shared")
        add_defines("CLAY_MOBILE")
        add_cflags("-Wall", "-Werror", "-O2", "-fPIC")
        set_targetdir(path.join("android/app/src/main/jniLibs", "$(ARCH)"))

        if get_config("renderer") == "sdl2" or get_config("renderer") == "sdl3" then
            add_includedirs(
                "vendor/cJSON", 
                "vendor/SDL/include",
                "vendor/SDL_image/include", 
                "vendor/SDL_ttf",
                "vendor/SDL2_gfx"
            )
            
            add_files("vendor/cJSON/cJSON.c")
            
            on_load(function (target)
                local arch = target:arch()
                local jniLibs_path = path.absolute(format("android/app/src/main/jniLibs/%s", arch))
                
                target:add("linkdirs", jniLibs_path)
                target:add("linkdirs", format("vendor/SDL2_gfx/build-android-%s", arch))
                
                target:add("links", "SDL2", "SDL2_gfx", "SDL2_image", "SDL2_ttf")
                target:add("links", "android", "log", "m")
                
                target:add("rpathdirs", jniLibs_path)
                target:add("shflags", "-shared", {force = true})
            end)
        elseif get_config("renderer") == "raylib" then
            add_links("raylib")
        end

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

    -- WASM platform configuration
    elseif is_plat("wasm") then
        set_kind("binary")
        set_targetdir("build/clay")
        set_toolset("cc", "emcc")
        set_toolset("ld", "emcc")
        add_defines("CLAY_WASM")
        
        add_cflags("-Wall", "-Werror", "-Os", "-DCLAY_WASM", "-mbulk-memory", "--target=wasm32")
        
        add_ldflags(
            "-Wl,--strip-all",
            "-Wl,--export-dynamic",
            "-Wl,--export=__heap_base",
            "-Wl,--export=ACTIVE_RENDERER_INDEX",
            "-s WASM=1",
            "-s USE_PTHREADS=0",
            "-s ASSERTIONS=1",
            "-s ALLOW_MEMORY_GROWTH=1",
            "-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']",
            "-s EXPORTED_FUNCTIONS=['_main','_printf','_measureTextFunction','_queryScrollOffsetFunction']",
            "-s STANDALONE_WASM",
            {force = true}
        )
        
        after_build(function (target)
            os.cp("index.html", "build/clay")
            os.cp("manifest.json", "build/clay")
            os.cp("fonts", "build/clay")
            os.cp("images", "build/clay")
            os.cp("scripts", "build/clay")
        end)

    -- Desktop platforms (Linux, macOS, Windows)
    else
        add_defines("CLAY_DESKTOP")
        set_toolset("cc", "clang")
        set_targetdir("build/clay")
        
        add_includedirs("vendor/cJSON") 
        add_files("vendor/cJSON/cJSON.c")
        
        add_cflags("-Wall", "-Werror", "-O2")
        add_cflags("-Wno-unused-variable")
        add_cflags("-Wno-missing-braces")

        if get_config("renderer") == "sdl2" or get_config("renderer") == "sdl3" then
            add_links("SDL2", "SDL2_image", "SDL2_ttf", "SDL2_gfx")
            add_includedirs("/usr/include/SDL2")
            add_defines("ROCKS_USE_SDL2")
        elseif get_config("renderer") == "raylib" then
            add_links("raylib")
            add_includedirs("/usr/include")
            add_defines("ROCKS_USE_RAYLIB")
        end
        
        add_links("m")
        
        after_build(function (target)
            os.cp("fonts", "build/clay")
            os.cp("images", "build/clay")
        end)
    end