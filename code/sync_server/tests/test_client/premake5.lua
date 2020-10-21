-- Copyright (c) NOMAD Group<nomad-group.net>

project("TestClient")
    kind("ConsoleApp")
    include_meta()
    files({
        "**.cpp",
        "**.h"
    })
    links({
        "enet",
        "fmtlib",
        "netlib",
        "database",
        "utility"
    })
    includedirs({
        ".",
        "../../../common",
        blu.extdir .. "/idasdk72/include",
        blu.extdir .. "/enet/include",
        blu.extdir .. "/flatbuffers/include",
        blu.extdir .. "/fmt/include",
        blu.extdir .. "/xenium"
    })