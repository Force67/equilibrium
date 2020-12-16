-- Copyright (c) NOMAD Group<nomad-group.net>

include("tools/build")

filter("architecture:x86_64")
    targetsuffix("_64")

filter ("configurations:Debug")
    defines("ND_DEBUG")

filter("configurations:Release")
    runtime("Release")
    optimize("Speed")

filter("configurations:Shipping")
    runtime("Release")
    optimize("Speed")
    flags({
        "LinkTimeOptimization"
    })

filter("language:C or C++")
    vectorextensions("SSE4.1")

filter("language:C++")
    cppdialect("C++17")

filter("system:windows")
    defines("OS_WIN")
    
workspace("RETK")
    configurations({
        "Debug",
        "Release",
        "Shipping"
    })
    flags {
        "MultiProcessorCompile"
    }
    defines("NOMINMAX")

    group("Dependencies")
    include("external")

    include("./code")