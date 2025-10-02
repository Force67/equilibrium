-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

local blu_profiling = dofile(path.join(_SCRIPT_DIR, "profiling.lua"))
local blu_os_cache = dofile(path.join(_SCRIPT_DIR, "os_cache.lua"))
blu_profiling.install()

-- Declare the blu build tool namespace
blu = {}
blu.profile = blu_profiling
blu.os_cache = blu_os_cache

include('family.lua')

if _ACTION == "vs2022" then
  require('vstudio')
end

-- if _ACTION == "gmake2" then
  local compile_commands = require("compile_commands/export-compile-commands") -- compile_commands.json support for clangd
  require("vscode/vscode") -- support for vscode
  local zed_debug = require("zed/debug") -- support for generating Zed debugger configs
-- end

-- pull in root dependencies
-- by order of importance.
include('os_extensions.lua')
include('sdk.lua')
include('build_config.lua')
include('platform_files.lua')
include('artifacts.lua')
include('format_extensions.lua')
include('build_info.lua')
include('dependency.lua')
include('component.lua')
include('unittest.lua')
  include('http_source.lua')

  newoption({
    trigger = "skip-compile-commands",
    description = "Skip generating compile_commands.json during gmake2 runs",
  })

  newoption({
    trigger = "skip-zed-debug",
    description = "Skip generating Zed debug configuration during gmake2 runs",
  })

  local gmake2_action = premake.action.get("gmake2")

  local function with_action(trigger, fn, ...)
    local previous = _ACTION
    _ACTION = trigger
    local ok, result = pcall(fn, ...)
    _ACTION = previous
    if not ok then
      error(result)
    end
    return result
  end

  premake.override(gmake2_action, "onWorkspace", function(base, wks)
    base(wks)

    if not _OPTIONS["skip-compile-commands"] then
      with_action("export-compile-commands", compile_commands.onWorkspace, wks)
    end

    if not _OPTIONS["skip-zed-debug"] then
      with_action("zed-debug", zed_debug.onWorkspace, wks)
    end
  end)
