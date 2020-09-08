-- Copyright (c) NOMAD Group<nomad-group.net>

include("tools/build")

filter("architecture:x86_64")
    targetsuffix("_64")

filter("configurations:Release")
    runtime("Release")
    optimize("Speed")

filter("language:C or C++")
    vectorextensions("AVX")  

workspace("NODA - Native")
    configurations({
        "Debug",
        "Release"
    })
    location("./build")
    architecture("x86_64")

    group("External")
    include("external")
    include("./code/client")

workspace("NODA - Managed")
    configurations({
        "Debug",
        "Release"
    })
    location("./managedbuild")
    include("external/managed.lua")
    include("./code/server")