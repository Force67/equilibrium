-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

project("Server")
    kind("StaticLib")
    targetname("server")
    files({
        "**.cpp",
        "**.h",
    })
    links({
        "network",
        "fmtlib",
        "database",
        "utility"
    })
    includedirs({
        ".",
        "../../common",
        blu.extdir .. "/idasdk72/include",
        blu.extdir .. "/enet/include",
        blu.extdir .. "/flatbuffers/include",
        blu.extdir .. "/fmt/include",
        blu.extdir .. "/sockpp/include",
        blu.extdir .. "/xenium"
    })