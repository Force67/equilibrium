-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

include("./build")

filter("architecture:x86_64")
    targetsuffix("_64")

filter("configurations:Debug")
    defines("TK_DBG")

filter("configurations:DebugAsan")
    flags({
        "NoRuntimeChecks", 
        "NoIncrementalLink"})
    editAndContinue("Off")
    enableASAN("true")
    flags("NoIncrementalLink")
    defines("TK_DBG")

filter("configurations:Release")
    runtime("Release")
    optimize("Speed")

filter("configurations:Shipping")
    runtime("Release")
    optimize("Full")
    flags({
        "LinkTimeOptimization"
    })
    defines("SHIPPING")

filter("language:C or C++")
    vectorextensions("SSE4.2")
    staticruntime("on")

filter("language:C++")
    cppdialect("C++20")

filter("system:windows")
    defines("OS_WIN")
filter{}

-- This is here rather temporarily
defines("PROJECT_NAME=\"%{prj.name}\"")    

workspace("Equilibrium")
    configurations({
        "Debug",
        "DebugAsan",
        "Release",
        "Shipping",
    })
    flags {
        "MultiProcessorCompile"
    }
    defines("NOMINMAX")
    -- external must always come first
    -- so 'dependencies' can link against them
    include("./external")
    include("./base")
    include("./components")
    include("./retk")
    include("./tools")