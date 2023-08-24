-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

-- Declare the blu build tool namespace
blu = {}

include('family.lua')

if _ACTION == "vs2022" then
  require('vstudio')
end

-- if _ACTION == "gmake2" then
  require("compile_commands/export-compile-commands") -- compile_commands.json support for clangd
  require("vscode/vscode") -- support for vscode
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