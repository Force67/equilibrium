-- Copyright (C) 2021 Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

component("ui")
    files({
        "**.cc",
        "**.h",
        "**.inl"
    })
    dependencies({
        "fmtlib",
        "glew",
        "glfw",
        "imgui",
    })
    links({
        "skia",
        -- operating system
        "opengl32",
        "user32",
        "gdi32",
        "shell32"
    })
    defines({
        "SK_GL"
    })
    includedirs({
        blu.extdir .. "/skia_sdk",
        blu.extdir .. "/skia_sdk/include",
    })
    libdirs({
        blu.extdir .. "/skia_sdk/lib/debug_static",
    })

group("Components/ui/test")
project("ui_test")
    kind("ConsoleApp")
    files({
        "test/run_all_tests.cc",
        "test/ui_test_suite.cc",
        "test/ui_test_suite.h",
        -- for now, just to get tests working...
        "gamma/*.cc",
        "gamma/*.h",
        "gamme/*.inl"
    })
    links({
        "base",
        "googlemock",
        "base",
        "ui",
    })
    includedirs({
        ".",
        "../../",
        blu.extdir .. "/ui",
        blu.extdir .. "/fmt/include",
        blu.extdir .. "/googletest/googletest/include",
    })