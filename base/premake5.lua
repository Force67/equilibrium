-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

group("Base")
project("base")
    kind("StaticLib")
    files({
        "**.cc",
        "**.h",
    })
    links({
        "fmtlib"
    })
    includedirs({
        ".",
        "../",
        blu.extdir .. "/fmt/include",
    })