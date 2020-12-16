-- Copyright (c) Force67 <github.com/Force67>.

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
        "storage",
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