-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

----  ENTER FILTER BARRIER: ONLY FILTERS AFTER THIS POINT  ----

	---- Architecture ----
filter("architecture:x86_64 or ARM64")
  targetsuffix("_64")
filter("architecture:x86")
  targetsuffix("_32")

	---- Supported Operating Systems ----
filter("system:windows")
  defines({
    "OS_WIN",
    "NOMINMAX"})
filter("system:linux")
  defines({
    "OS_LINUX",
    "OS_POSIX"}) -- we also define the POSIX alias here)
filter("system:macosx")
  defines("OS_MACOS")
filter{}

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

  ---- Build configurations ----
filter("configurations:Debug")
  defines("CONFIG_DEBUG")

filter("configurations:DebugAsan")
flags({
  "NoRuntimeChecks",
  "NoIncrementalLink"})
  defines("CONFIG_DEBUG")
  editAndContinue("Off")

filter({"kind:ConsoleApp OR WindowedApp", "configurations:DebugAsan"})
  enableASAN("true")
filter{}

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

----  LEAVE FILTER BARRIER: NO FILTERS AFTER THIS POINT  ----
filter{}

-- This is here rather temporarily
defines("PROJECT_NAME=\"%{prj.name}\"")