-- Copyright (c) NOMAD Group<nomad-group.net>

project("database")
    kind("StaticLib")
    files({
        "**.cpp",
        "**.h",
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