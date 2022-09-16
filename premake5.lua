-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.
require("premake", ">=5.0-beta3")

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
  include("./external")

  --profiler.start()
  include("./base")
  --profiler.stop()

  --profiler.start()
  include("./components")
  --profiler.stop()

  --profiler.start()
  include("./retk")
  --profiler.stop()

  --profiler.start()
  include("./nemisis")
  --profiler.stop()
  
  -- optional
  opt_include("./velocity")
  opt_include("./Avalanche")

  include("./tools")