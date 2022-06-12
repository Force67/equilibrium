-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

group("Components/sync")
project("sync")
    kind("StaticLib")
    files({
        "**.cc",
        "**.h",
    })
    links({
        "database",
        "network"
    })
    includedirs({
        ".",
        "../",
        "../../",
        blu.extdir .. "/sqlite",
        blu.extdir .. "/fmt/include",
        blu.extdir .. "/flatbuffers/include",
        blu.extdir .. "/sockpp/include",
        blu.extdir .. "/xenium",
    })