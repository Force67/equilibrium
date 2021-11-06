-- Copyright (C) 2021 Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

group("Apps")
project("App_Main")
    kind("WindowedApp")
    include_meta()
    files({
        "retk_full/retk_full.rc",
        "retk_full/retk.manifest",
        "retk_full/**.cc",
        "retk_full/**.h"
    })
    dependencies({
        "fmtlib",
        "glew",
        "glfw",
        "imgui",

        -- system
        "ui",
    })
    links({
        -- third party
        "skia",
        -- operating system
        "opengl32",
        "user32",
        "gdi32",
        "shell32",
        "Shcore"
    })
    linkoptions({
        -- use our regular main instead of winmain
        "/ENTRY:mainCRTStartup"
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
    })
    libdirs({
        blu.extdir .. "/skia_sdk/lib/debug_static",
    })