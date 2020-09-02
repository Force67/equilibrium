-- Copyright (c) NOMAD Group<nomad-group.net>

include("tools/build")

filter("architecture:x86_64")
    targetsuffix("_64")

workspace("NODA")
    configurations({
        "Debug",
        "Release"
    })
    architecture("x86_64")  

    location("./build")
    include("external")
    include("./code/client")
