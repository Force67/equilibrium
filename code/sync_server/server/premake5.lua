-- Copyright (c) NOMAD Group<nomad-group.net>

project("Server")
    kind("StaticLib")
    targetname("server")
    files({
        "**.cpp",
        "**.h",
    })
    links({
        "enet",
        "fmtlib"
    })
    includedirs({
        ".",
        blu.extdir .. "/idasdk72/include",
        blu.extdir .. "/enet/include",
        blu.extdir .. "/flatbuffers/include",
        blu.extdir .. "/fmt/include",
    })