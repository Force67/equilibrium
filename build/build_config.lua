-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

---- Supported Operating Systems ----
when({'system:windows'}, function()
  defines({
    "OS_WIN",
    "NOMINMAX"})
end)

when({'system:linux'}, function()
  defines({
    "OS_LINUX",
    "OS_POSIX"}) -- we also define the POSIX alias here)
end)

when({'system:macosx'}, function()
  defines("OS_MACOS")
end)

---- Build configurations ----

-- recommended for development
-- has no optimization
when({'configurations:Debug'}, function()
  --runtime("Debug")
  defines({
    "CONFIG_DEBUG",
    "_DEBUG" -- enable MSVC debug features, such as debug heap
  })
  --targetsuffix("_%{cfg.architecture}_d")
end)

-- special configuration for memory debugging
-- enables ASAN and msvc debug heap
when({'configurations:DebugMem'}, function()
  --runtime("Debug")
  --flags({
  --  "NoRuntimeChecks",
  --  "NoIncrementalLink"})
  defines("CONFIG_DEBUG")
  --editAndContinue("Off")
  defines({
    "CONFIG_DEBUG",
    "_DEBUG" -- enable MSVC debug features, such as debug heap
  })
  --targetsuffix("_%{cfg.architecture}_dm")
end)

when({'kind:ConsoleApplication or WindowedApplication', 'configurations:DebugMem'}, function()
  enableASAN("true")
end)

-- release build, good code gen, but no sub folding
when({'configurations:Release'}, function()
  --runtime("Release")
  --optimize("Speed")
  defines("CONFIG_RELEASE")
  --targetsuffix("_%{cfg.architecture}_r")
end)

-- this mode features release codegen but with profiler logging (usually using tracy)
-- enabled
when({'configurations:Profile'}, function()
  --runtime("Release")
  --optimize("Speed")
  defines({
    "CONFIG_RELEASE", 
    "ENABLE_PROFILE"})
  --targetsuffix("_%{cfg.architecture}_p")
end)

-- fully optimized build, ready to be shipped to the user
when({'configurations:Shipping'}, function()
  --runtime("Release")
  --optimize("Full") https://stackoverflow.com/questions/5063334/what-is-the-difference-between-the-ox-and-o2-compiler-options
  -- read also: https://github.com/ulfjack/ryu/pull/70#issuecomment-412168459 for further info on why we use O2
  --optimize("Speed")
  --flags({
  --  "LinkTimeOptimization"
  --})
  defines("CONFIG_SHIPPING")
  --targetsuffix("")
end)

--when({'language:C or C++'}, function()
  --vectorextensions("SSE4.2")
  --vectorextensions("AVX2")
  --staticruntime("on")
  -- disable exceptions
  exceptionHandling("Off")
  -- all warnings (/wall)
  -- warnings("Everything")
  -- all errors are warnings
  -- includes very pedantic warnings
  --flags("FatalWarnings")
  -- no rtti type info
  rtti("Off")
--end)

--when({'language:C++'}, function()
  --cppDialect("C++20")
--end)

-- This is here rather temporarily
defines("PROJECT_NAME=\"%{prj.name}\"")