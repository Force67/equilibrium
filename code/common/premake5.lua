-- Copyright (c) NOMAD Group<nomad-group.net>

project("Common")
    kind("StaticLib")
    targetname("common")
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