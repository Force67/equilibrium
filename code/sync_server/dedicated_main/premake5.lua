-- Copyright (c) NOMAD Group<nomad-group.net>

project("CxxServer")
    kind("ConsoleApp")
    targetname("Noda_CXX")
    include_meta()
    files({
        "**.cpp",
        "**.h",
        "Noda_Server.rc",
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