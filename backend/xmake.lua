add_rules("mode.debug", "mode.release")

-- Add libraries from: https://github.com/xmake-io/xmake-repo/tree/dev/packages
add_requires("drogon", "inja", "picosha2")

target("backend")
    set_kind("binary")
    set_group("all")
    
    set_languages("cxx17")
    
    add_headerfiles("src/*.hpp")
    add_files("src/*.cpp")

    add_packages("drogon", "inja", "picosha2")
    add_defines("NOMINMAX") -- For inja compilation on Windows

    local outputdir = get_config("outputdir")
    if outputdir ~= nil then
        set_targetdir(outputdir)
    end

    after_build_files(function (target) 
        import("core.project.config")
        import("core.base.json")
        config.load()
        local outputdir = get_config("outputdir") or path.join(os.projectdir(), config.get("buildir"), config.get("plat"), config.get("arch"), config.get("mode"))
        local config_table = json.loadfile(path.join(os.scriptdir(), "settings", "config.json"))
        config_table["app"]["document_root"] = path.join(outputdir, "public")
        local config_json = json.encode(config_table)
        io.writefile(path.join(outputdir, "settings", "config.json"), config_json)
    end)