-- Copyright (c) NOMAD Group<nomad-group.net>

include("tools/build")

filter("architecture:x86_64")
    targetsuffix("_64")

filter("configurations:Release")
    runtime("Release")
    optimize("Speed")

filter("language:C or C++")
    architecture("x86_64")
    vectorextensions("AVX")  

workspace("NODA")
    configurations({
        "Debug",
        "Release"
    })
    location("./build")

    group("External")
    include("external")

    group("NODA")
    include("./code/client")
    include("./code/server")
