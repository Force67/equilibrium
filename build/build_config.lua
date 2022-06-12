-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

-- ENTER FILTER BARRIER: ONLY FILTERS AFTER THIS POINT

filter("architecture:x86_64 or ARM64")
  targetsuffix("_64")
filter("architecture:x86")
  targetsuffix("_32")

-- all supported operating systems must be listed below:
filter("system:windows")
  defines("OS_WIN")
filter("system:linux")
  defines({
    "OS_LINUX",
    "OS_POSIX"}) -- we also define the POSIX alias here
filter("system:macosx")
  defines("OS_MACOS")

filter("language:C or C++")
  -- disable exceptions
  exceptionhandling("Off")
  -- all warnings (/wall)
  -- warnings("Everything")
  -- all errors are warnings
  -- includes very pedantic warnings
  --flags("FatalWarnings")
  -- no rtti type info
  rtti("Off")

filter("configurations:Debug")
  defines("CONFIG_DEBUG")

filter("configurations:DebugAsan")
flags({
  "NoRuntimeChecks",
  "NoIncrementalLink"})
  editAndContinue("Off")
  enableASAN("true")
  flags("NoIncrementalLink")
  defines("CONFIG_DEBUG")

filter("configurations:Release")
  runtime("Release")
  optimize("Speed")
  defines("CONFIG_RELEASE")

filter("configurations:Shipping")
  runtime("Release")
  optimize("Full")
  flags({
    "LinkTimeOptimization"
  })
  defines("CONFIG_SHIPPING")

filter("language:C or C++")
  vectorextensions("SSE4.2")
  staticruntime("on")

filter("language:C++")
  cppdialect("C++20")

-- LEAVE FILTER BARRIER
-- NO FILTERS BEYOND THIS POINT
filter{}

-- This is here rather temporarily
defines("PROJECT_NAME=\"%{prj.name}\"")
