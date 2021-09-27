-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

group("Components/tilted_reflection")
project("tilted_reflection")
    kind("StaticLib")
    files({
        "*.cc",
        "*.h",
    })
    links("googlemock")
    includedirs({
        ".",
        "../",
        "../../",
        blu.extdir .. "/rapidjson/include",
        blu.extdir .. "/fmt/include",
        blu.extdir .. "/googletest/googletest/include",
    })

group("Components/tilted_reflection/test")
project("tilted_reflection_test")
    kind("ConsoleApp")
    include_meta()
    files({
        "test/test_runner.cc",
        "*.cc",
        "*.h"
    })
    links({"base", "googlemock"})
    includedirs({
        ".",
        "../",
        "../../",
        blu.extdir .. "/rapidjson/include",
        blu.extdir .. "/fmt/include",
        blu.extdir .. "/googletest/googletest/include",
    })