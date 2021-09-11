-- Copyright (C) 2021 Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

group("Components/ui")
project("ui")
    kind("StaticLib")
    files({
        "**.cc",
        "**.h",
        "**.inl"
    })
    links({
        "fmtlib",
        "skia",
        "glew",
        "glfw",
        "imgui",
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
        ".",
        "../../",
        "../../retk",
        blu.extdir .. "/skia_sdk",
        blu.extdir .. "/skia_sdk/include",
        blu.extdir .. "/sockpp/include",
        blu.extdir .. "/flatbuffers/include",
        blu.extdir .. "/fmt/include",
        blu.extdir .. "/glew/include",
        blu.extdir .. "/glfw/include",
        blu.extdir .. "/imgui",
    })
    libdirs({
        blu.extdir .. "/skia_sdk/lib/debug_static",
    })
    filter("system:windows")
        defines("GLFW_EXPOSE_NATIVE_WIN32")

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