-- Project config
set_project("MyQuest")
set_version("1.0")
set_languages("c99")
add_rules("mode.debug", "mode.release")

add_includedirs("include")

includes("xmake/android_sdl.lua")    

-- Android platform configuration
if is_plat("android") then
    add_defines("CLAY_MOBILE")
    add_includedirs("vendor/cJSON", 
                    "vendor/clay",
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


if is_plat("wasm") then
    set_toolset("cc", "emcc")
    set_toolset("ld", "emcc")

    add_defines("CLAY_WASM")
    add_includedirs("vendor/clay")
    add_headerfiles("vendor/clay/clay.h")
    add_files("src/**.c")
    remove_files("src/platforms/sdl/**.c")
    
    set_kind("binary")
    set_targetdir("build/clay")
    
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
        "-s EXPORTED_FUNCTIONS=['_main','_printf']",
        "-o build/clay/index.wasm",  -- Output only WASM file
        {force = true}
    )
    
    after_build(function (target)
        os.cp("index.html", "build/clay")
        os.cp("manifest.json", "build/clay")
        os.cp("fonts", "build/clay")
        os.cp("images", "build/clay")
    end)
end

if is_plat("linux", "macosx", "windows") then
    add_defines("CLAY_DESKTOP")
    
    -- For Linux cross-compilation
    if is_plat("linux") then
        set_toolset("cc", "clang")
    end
    
    add_includedirs("vendor/cJSON") 
    add_files("vendor/cJSON/cJSON.c")
    add_includedirs("vendor/clay") 
    
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
        -- Set the target to a binary (WASM file)
        set_kind("binary")
        set_targetdir("build/clay")
        
        -- Add linker flags for WASM
        add_ldflags(
            "-s WASM=1", 
            "-s USE_PTHREADS=0", 
            "-s ASSERTIONS=1", 
            "-s ALLOW_MEMORY_GROWTH=1",
            "-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']",
            "-s EXPORTED_FUNCTIONS=['_main','_printf','_measureTextFunction','_queryScrollOffsetFunction']",
            "-s STANDALONE_WASM",  -- Ensure only a .wasm file is generated
            {force = true}
        )
        
        -- Copy assets after build
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



