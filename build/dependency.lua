-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.
-- Implement dependency extension model for premake.

local p = premake
local api = p.api


-- TODO: on a per project config.
-- so we can support different configurations
-- e.g scope = config

-- Use this to import a required dependency into your project
-- it will parse public and private includes
api.register {
    name = "dependencies",
    scope = "config",
    kind = "list:mixed",
    tokens = true,
}

-- The Following functions automatically export given params
-- through the 'dependencies' function:
api.register {
    name = "publicincludes",
    scope = "config",
    kind = "list:directory",
    tokens = true,
}

api.register {
    name = "publiclinks",
    scope = "config",
    kind = "list:directory",
    tokens = true,
}

api.register {
    name = "publicdefines",
    scope = "config",
    kind = "list:directory",
    tokens = true,
}

function precook_configs(prj) 
    local cfgs = table.fold(prj.configurations or {}, prj.platforms or {})
    premake.oven.bubbleFields(prj, prj, cfgs)
    return premake.oven.bakeConfigList(prj, cfgs)
end

function remoteeachconfig(prj)
    local configs = precook_configs(prj)
    local count = #configs

    -- Once the configurations are mapped into the workspace I could get
    -- the same one multiple times. Make sure that doesn't happen.
    local seen = {}

    local i = 0
    return function ()
        i = i + 1
        if i <= count then
            local cfg = premake.project.getconfig(prj, configs[i][1], configs[i][2])
            if not seen[cfg] then
                seen[cfg] = true
                return cfg
            else
                i = i + 1
            end
        end
    end
end

function remotematchconfig(prj, srcname)
    for rcfg in remoteeachconfig(prj) do
        if (rcfg.name == srcname) then
            return rcfg
        end
    end

    return nil
end

-- dependency -> getpublicdir
premake.override(premake.project, "bake", function(oldfn, prj)
    -- bake the regular content first
    oldfn(prj)
    -- merge dependencies
    for cfg in premake.project.eachconfig(prj) do
        -- merge project links
        for i = 1, #cfg.dependencies do
            local d = cfg.dependencies[i]
            table.insert(cfg.links, d)

            local rp = premake.workspace.findproject(cfg.workspace, d)
            if rp then
                -- does the remote project follow a similar rule, if so
                -- try matching it.
                local rcfg = remotematchconfig(rp, cfg.name)
                if rcfg then
                    -- Merge public includes
                    for j = 1, #rcfg.publicincludes do
                        local rpub = rcfg.publicincludes[j]
                        table.insert(cfg.includedirs, rpub)
                        --print("For " .. rcfg.name .. "RPUB IS " .. rpub)
                    end

                    -- Merge public defines
                    for j = 1, #rcfg.publicdefines do
                        local rdef = rcfg.publicdefines[j]
                        table.insert(cfg.defines, rdef)
                        --print("For " .. rcfg.name .. "RDEF IS " .. rdef)
                    end
                else
                    print("Failed to match remote config (Source proj is " .. prj.name .. 
                    " dependency proj is " .. rp.name .. ")")
                end
            end
        end
    end
end)