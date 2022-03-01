-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.
-- This represents the root of the build directory

-- Declare the blu build tool namespace
blu = {}

require('vstudio')

-- pull in root dependencies
-- by order of importance.
include('asan_support.lua')
include('build_config.lua')
include('artifacts.lua')
include('build_info.lua')
include('dependency.lua')
include('component.lua')