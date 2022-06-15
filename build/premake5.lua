-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

-- Declare the blu build tool namespace
blu = {}

require('vstudio')

-- pull in root dependencies
-- by order of importance.
include('sanitizer.lua')
include('build_config.lua')
include('platform_files.lua')
include('artifacts.lua')
include('build_info.lua')
include('dependency.lua')
include('component.lua')
include('unittest.lua')