local outputdir = path.join(path.join(os.scriptdir(), "build"))
set_config("outputdir", outputdir)

includes("backend/xmake.lua", "frontend/xmake.lua")

--[[
    Use the following command to build:
        xmake build fullstack
    or
        xmake -g all
]]

target("fullstack")
    set_kind("phony")
    set_default(true)
    add_deps("backend", "frontend")
