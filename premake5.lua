-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.
require("premake", ">=5.0-beta3")

include("./build")

workspace("Equilibrium")
  configurations({
    "Debug",
    "DebugAsan",
    "Release",
    "Shipping",
  })
  flags({
    "MultiProcessorCompile"
  })
  defines("NOMINMAX")
  -- external must always come first
  -- so 'dependencies' can link against them
  include("./external")
  include("./base")
  include("./components")
  include("./retk")
  include("./tools")