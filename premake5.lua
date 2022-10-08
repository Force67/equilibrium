-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.
require("premake", ">=5.0-beta3")
-- pull in the blu.build tool
include("./build")

workspace("Equilibrium")
  configurations({
    "Debug",
    "DebugAsan",
    "Release",
    "Profile",
    "Shipping",
  })
  flags({
    "MultiProcessorCompile"
  })
  -- external must always come first
  -- so 'dependencies' can link against them
  family("equilibrium")
  grouped_include("./external", "Dependencies")
  grouped_include("./base", "Base")
  grouped_include("./components", "Components")
  family("")

  include("./projects")