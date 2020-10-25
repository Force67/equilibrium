-- Copyright (c) NOMAD Group<nomad-group.net>

project("DbModel")
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
        "utility",
        "GoogleMock"
    })
    includedirs({
        ".",
        "../../common",
        blu.extdir .. "/idasdk72/include",
        blu.extdir .. "/enet/include",
        blu.extdir .. "/flatbuffers/include",
        blu.extdir .. "/googletest/googlemock/include",
        blu.extdir .. "/googletest/googletest/include",
        blu.extdir .. "/fmt/include",
        blu.extdir .. "/xenium"
    })