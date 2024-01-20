target("frontend")
    set_kind("phony")
    set_group("all")

    add_extrafiles("resources/**")
    add_extrafiles("package.json")
    add_extrafiles("webpack.config.js")

    -- Using this may cause problems with xmake rules
    on_build(function (target)
        local frontend_dir = path.join(path.directory(os.scriptdir()), "frontend")
        local args = {"--prefix", frontend_dir}
        os.execv("pnpm", table.join(args, {"i"}))
        local build_cmd = table.join(args, {"run", "build"})
        local outputdir = get_config("outputdir")
        if outputdir ~= nil then
            table.insert(build_cmd, "--output-path")
            table.insert(build_cmd, path.join(outputdir, "public", "app"))
        end
        os.execv("pnpm", build_cmd)
        local inertia_html = path.join(frontend_dir, "resources", "Inertia.html")
        if outputdir ~= nil then
            os.cp(inertia_html, path.join(outputdir, "Inertia.html"))
        else
            os.cp(inertia_html, path.join(frontend_dir, "build", "Inertia.html"))
        end
    end)
