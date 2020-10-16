-- Copyright (c) NOMAD Group<nomad-group.net>

project("netstack")
    kind("StaticLib")
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
        blu.extdir .. "/enet/include",
        blu.extdir .. "/flatbuffers/include",
        blu.extdir .. "/fmt/include",
    })