-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

group("Base")
project("base")
    kind("StaticLib")
    files({
        "**.cc",
        "**.h",
        "**.in",
        "**.inl"
    })
    links({
        "fmtlib"
    })
    defines("BASE_IMPLEMENTATION")
    includedirs({
        ".",
        "../",
        blu.extdir .. "/xenium",
        blu.extdir .. "/fmt/include",
    })