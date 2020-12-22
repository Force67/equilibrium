-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

group("Components")

project("sync")
    kind("StaticLib")
    files({
        "*.cpp",
        "*.h",
    })
    links({
        "database",
        "network"
    })
    includedirs({
        "../",
        ".",
        blu.extdir .. "/sqlite",
        blu.extdir .. "/fmt/include",
    })