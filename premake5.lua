-- Copyright (C) 2022 Vincent Hengel.
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
  -- external must always come first
  -- so 'dependencies' can link against them
  include("./external")
  include("./base")
  include("./components")
  include("./retk")
  include("./nemisis")

  -- if v lang was installed, include it
  if os.isdir("./velocity") then
    include("./velocity")
  end

  include("./tools")
