-- Copyright (c) NOMAD Group<nomad-group.net>

project("utility")
    kind("StaticLib")
    files({
        "**.cpp",
        "**.h",
    })
    links({
        "fmtlib"
    })
    includedirs({
        ".",
        blu.extdir .. "/fmt/include",
    })