-- Copyright (C) 2023 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

function include(path)
  if not string.find(path, ".lua") then
    -- https://github.com/premake/premake-core/blob/6.x/core/modules/_G/_G.lua#L16
    doFile(path .. "/premake6.lua")
  else
    doFile(path)
  end
end

include("./build")

workspace('Equilibrium', function()
  configurations({
    "Debug",
    "DebugAsan",
    "Release",
    "Profile",
    "Shipping",
  })
  --flags({
  --  "MultiProcessorCompile"
  --})
  -- external must always come first
  -- so 'dependencies' can link against them
  --include("./external")
  --include("./base")
  --include("./components")
  --include("./retk")
  --include("./nemisis")
  --opt_include("./velocity")
  --opt_include("./Avalanche")

  --include("./tools")
end)