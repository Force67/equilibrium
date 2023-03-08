-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.
-- Implement dependency extension model for premake.
local p = premake
local api = p.api

-- Use this to import a required dependency into your project
-- it will parse public and private includes
api.register({
  name = "dependencies",
  scope = "config",
  kind = "list:mixed",
  tokens = true
})

-- The Following functions automatically export given params
-- through the 'dependencies' function:
api.register({
  name = "pubincludedirs",
  scope = "config",
  kind = "list:directory",
  tokens = true
})

api.register({
  name = "pubdefines",
  scope = "config",
  kind = "list:string",
  tokens = true
})

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
  return function()
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
    if (rcfg.name == srcname) then return rcfg end
  end

  return nil
end

function merge_pubdirs(src_cfg, target_config)
  for j = 1, #src_cfg.pubincludedirs do
    local rpub = src_cfg.pubincludedirs[j]
    table.insert(target_config.includedirs, rpub)
    -- print("For " .. src_cfg.name .. " pub dir is " .. rpub)
  end
end

function merge_pubdefs(src_cfg, target_config)
  for j = 1, #src_cfg.pubdefines do
    local rdef = src_cfg.pubdefines[j]
    table.insert(target_config.defines, rdef)
    -- print("For " .. rcfg.name .. "RDEF IS " .. rdef)
  end
end

-- dependency -> getpublicdir
premake.override(premake.project, "bake", function(oldfn, prj)
  -- bake the regular content first
  oldfn(prj)
  -- merge dependencies
  for cfg in premake.project.eachconfig(prj) do
    -- public = private for source project in order to
    -- avoid code duplication
    merge_pubdirs(cfg, cfg)
    merge_pubdefs(cfg, cfg)

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
          merge_pubdirs(rcfg, cfg)
          merge_pubdefs(rcfg, cfg)
        else
          error("Failed to match remote config (Source proj is " .. prj.name ..
                  " dependency proj is " .. rp.name .. ")")
        end
      end
    end

    -- TODO: this doesnt really belong in a 'bake' step....
    if cfg.sdktargetdir then
      -- copy sdk files to target dir
      for i = 1, #cfg.sdkfiles do
        local source = cfg.sdkfiles[i]
        local dest = cfg.targetdir .. "/" .. cfg.sdktargetdir .. "/" .. path.getrelative(prj.basedir, source)
        local targetdir = path.getdirectory(dest)
        ok, err = os.mkdir(targetdir) -- https://github.com/premake/premake-core/blob/b084bea561d18f1a84b488430bf7861214c3923a/src/base/os.lua#L450

        if not ok then
          error("Failed to create directory " .. targetdir .. " " .. err)
        end

        -- delete files from former runs
        if os.isfile(dest) then
          os.remove(dest)
        end

        ok, err = os.copyfile(source, dest)
        if not ok then
          error("Failed to copy " .. source .. " to " .. destination .. " " .. err)
        end
      end
    end
  end
end)
