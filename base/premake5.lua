-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

local function base_project()
    dependencies("googlemock")
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
        blu.rootdir,
        blu.extdir .. "/fmt/include",
    })
end

group("Base")
project("base")
    kind("StaticLib")
    base_project()

-- since default unittests from the component model link against base, 
-- we have to roll this stuff in our own way, for now...
project("base_unittests")
    kind("ConsoleApp")
    base_project()