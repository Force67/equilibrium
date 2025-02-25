-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

----  ENTER FILTER BARRIER: ONLY FILTERS AFTER THIS POINT  ----

---- Supported Operating Systems ----
filter("system:windows")
defines({
    "OS_WIN",
    "NOMINMAX" })
buildoptions("/utf-8") -- treat everything as utf-8
filter("system:linux")
defines({
    "OS_LINUX",
    "OS_POSIX" })               -- we also define the POSIX alias here)
buildoptions("-mwaitpkg -mrtm") -- Enable Intel(R) Transactional Synchronization Extensions (-mrtm) and WAITPKG instructions support (-mwaitpkg) on relevant processors
defines("OS_MACOS")

filter {}
-- -std=c++2b

---- Build configurations ----

-- recommended for development
-- has no optimization
filter("configurations:Debug")
runtime("Debug")
symbols("On")
defines({
    "CONFIG_DEBUG",
    "_DEBUG" -- enable MSVC debug features, such as debug heap
})
targetsuffix("_%{cfg.architecture}_d")
filter {}

-- special configuration for memory debugging
-- enables ASAN and msvc debug heap
filter("configurations:DebugMem")
runtime("Debug")
symbols("On")
flags({
    "NoRuntimeChecks",
    "NoIncrementalLink" })
defines("CONFIG_DEBUG")
editAndContinue("Off")
defines({
    "CONFIG_DEBUG",
    "_DEBUG" -- enable MSVC debug features, such as debug heap
})
targetsuffix("_%{cfg.architecture}_dm")
filter {}

filter({ "kind:ConsoleApp OR WindowedApp", "configurations:DebugMem" })
sanitize("Address")
filter {}

-- release build, good code gen, but no sub folding
filter("configurations:Release")
runtime("Release")
optimize("Speed")
defines("CONFIG_RELEASE")
targetsuffix("_%{cfg.architecture}_r")
filter {}

-- this mode features release codegen but with profiler logging (usually using tracy)
-- enabled
filter("configurations:Profile")
runtime("Release")
optimize("Speed")
defines({
    "CONFIG_RELEASE",
    "ENABLE_PROFILE" })
targetsuffix("_%{cfg.architecture}_p")
filter {}

-- fully optimized build, ready to be shipped to the user
filter("configurations:Shipping")
runtime("Release")
symbols("On")
--optimize("Full") https://stackoverflow.com/questions/5063334/what-is-the-difference-between-the-ox-and-o2-compiler-options
-- read also: https://github.com/ulfjack/ryu/pull/70#issuecomment-412168459 for further info on why we use O2
optimize("Speed")
flags({
    "LinkTimeOptimization"
})
defines("CONFIG_SHIPPING")
targetsuffix("")
filter {}

filter("language:C or C++")
--vectorextensions("SSE4.2")
vectorextensions("AVX2") -- 10- 15% faster than SSE4.2, even 10 year old CPUs support it
staticruntime("on")
-- disable exceptions
exceptionhandling("Off")
-- all warnings (/wall)
-- warnings("Everything")
-- all errors are warnings
-- includes very pedantic warnings
--flags("FatalWarnings")
-- no rtti type info
rtti("Off")
justmycode("Off")
pic("On") -- generate position independent code

filter("language:C++")
cppdialect("C++20")

filter { "language:C++", "system:linux" }
buildoptions("-std=c++2b")

----  LEAVE FILTER BARRIER: NO FILTERS AFTER THIS POINT  ----
filter {}

-- This is here rather temporarily
defines("PROJECT_NAME=\"%{prj.name}\"")
