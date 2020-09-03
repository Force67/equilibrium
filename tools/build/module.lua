-- Copyright (c) Stronkat. All rights reserved.
-- Purpose: Build script for engine/game modules.

local p = premake

blu.module = p.api.container("module", p.workspace)
local mod = blu.module

function mod.new(name)
    local m = p.container.new(blu.module, name)

    group("Modules")

    -- create the project
    m.project = project(name)

    filter("Configurations:Ship")
        kind("StaticLib")
    filter("Configurations:not Ship")
        kind("SharedLib")
    filter{}

    return m
end

p.api.register({
    name = "dependencies",
    scope = "module",
    kind = "list:string",
    tokens = true,
})