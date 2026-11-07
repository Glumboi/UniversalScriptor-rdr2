add_rules("mode.debug", "mode.release")

set_runtimes("MD")

add_requires("minhook")
add_requires("lua")

target("GlumityLib")
    set_kind("static")
    add_files("src/GlumityLib/*.c")
    add_links("user32")
    add_packages("minhook")
    
    after_build(function (target)
        local dest_dir = "src/TCCBridge-rdr2/default_bridge_env/tcc_include/include/GlumityLib"
        os.mkdir(dest_dir)
        os.cp("src/GlumityLib/*.h", dest_dir)
        os.cp("src/GlumityLib/*.c", dest_dir)
        os.cp("src/GlumityLib/*.asm", dest_dir)
        print("-> GlumityLib source files successfully backed up to TCC environment.")
    end)

target("TCCBridge-rdr2")
    set_kind("shared")
    add_files("src/TCCBridge-rdr2/*.cpp")
    add_files("src/TCCBridge-rdr2/include/Scipthookrdr2/*.c")
    add_linkdirs("src/TCCBridge-rdr2/lib")
    add_links("user32", "Gdi32", "libtcc", "ScriptHookRDR2") -- Grouped links together
    add_deps("GlumityLib")
    add_packages("minhook")
    set_languages("c++20")
    add_includedirs("src/GlumityLib")
    add_includedirs("src/TCCBridge-rdr2/include")
    
    -- Copy TCCBridge-rdr2.dll, libtcc.dll, and set up environment folders in build/Plugins
    after_build(function (target)
        local dest_plugins = "build/Plugins"
        os.mkdir(dest_plugins)

        -- Copy the compiled TCCBridge-rdr2.dll from xmake's output directory to build/Plugins
        os.cp(target:targetfile(), dest_plugins)

        -- Copy libtcc.dll
        if os.isfile("src/TCCBridge-rdr2/lib/libtcc.dll") then
            os.cp("src/TCCBridge-rdr2/lib/libtcc.dll", dest_plugins)
        elseif os.isfile("libtcc.dll") then
            os.cp("libtcc.dll", dest_plugins)
        end

        -- Ensure jit_scripts and default_bridge_env exist in build/Plugins
        os.mkdir(path.join(dest_plugins, "jit_scripts"))
        
        if os.isdir("src/TCCBridge-rdr2/default_bridge_env") then
            os.cp("src/TCCBridge-rdr2/default_bridge_env", dest_plugins)
        end
        
        print("-> TCCBridge-rdr2 and its files successfully placed in build/Plugins.")
    end)

target("version")
    set_kind("shared")
    set_filename("version.dll")
    add_files("src/versionDllInjector/*.cpp")
    add_links("user32")
    add_linkdirs("build")
    add_includedirs("src/GlumityLib")
    add_packages("minhook")
    add_deps("GlumityLib")
    set_languages("c++20")

    -- Copy version.dll directly to the root of the build folder
    after_build(function (target)
        local dest_root = "build"
        os.mkdir(dest_root)
        os.cp(target:targetfile(), dest_root)
        print("-> version.dll successfully placed in build root.")
    end)