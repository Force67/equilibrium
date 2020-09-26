-- Copyright (c) NOMAD Group<nomad-group.net>

include("tools/build")

filter("architecture:x86_64")
    targetsuffix("_64")

filter("configurations:Release")
    runtime("Release")
    optimize("Speed")

filter("language:C or C++")
    vectorextensions("SSE4.1")

filter("language:C++")
    cppdialect("C++17")
    
workspace("NODA")
    configurations({
        "Debug",
        "Release"
    })
    group("Dependencies")
    include("external")

    group("Project")
    include("./code/client")
    include("./code/server")