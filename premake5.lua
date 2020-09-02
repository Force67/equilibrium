-- Copyright (c) NOMAD Group<nomad-group.net>

include("tools/build")

filter("architecture:x86_64")
    targetsuffix("_64")

filter("configurations:Release")
    runtime("Release")
    optimize("Speed")

workspace("NODA")
    configurations({
        "Debug",
        "Release"
    })
    architecture("x86_64")
    vectorextensions("AVX")  

    location("./build")
    include("external")
    include("./code/client")
