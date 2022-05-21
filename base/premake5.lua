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
    removefiles("**_test.cc")

-- base is a special case where we cannot rely on the unittest model
-- found in components, so we have to manually add the test project
project("base_unittests")
    kind("ConsoleApp")
    base_project()
    removefiles("allocator/**_test.cc")

project("base_memory_unittests")
    kind("ConsoleApp")
    base_project()
    