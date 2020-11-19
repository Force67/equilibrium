-- Copyright (c) NOMAD Group<nomad-group.net>

project("database")
    kind("StaticLib")
    files({
        "*.cpp",
        "*.h",
    })
    links({
        "sqlite"
    })
    includedirs({
        ".",
        "../moc_protocol",
        blu.extdir .. "/sqlite",
        blu.extdir .. "/flatbuffers/include",
        blu.extdir .. "/fmt/include",
    })

project("db_model")
    kind("ConsoleApp")
    include_meta()
    files({
        "tests/db_model/*.cpp",
        "tests/db_model/*.h"
    })
    links({
        "fmtlib",
        "database",
        "utility"
    })
    includedirs({
        ".",
        "../",
        blu.extdir .. "/idasdk72/include",
        blu.extdir .. "/enet/include",
        blu.extdir .. "/flatbuffers/include",
        blu.extdir .. "/googletest/googlemock/include",
        blu.extdir .. "/googletest/googletest/include",
        blu.extdir .. "/fmt/include",
        blu.extdir .. "/xenium"
    })