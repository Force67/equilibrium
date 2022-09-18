-- Copyright (C) 2021 Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

project("App_Main")
    kind("WindowedApp")
    include_meta()
    --include_ui()
    files({
        "retk.manifest",
        "**.cc",
        "**.h"
    })
    dependencies({
        "fmtlib",
        "imgui",
        -- system
        "ui",
        "gpu",
        "program_loader"
    })
    dependencies("tracysdk")
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
        --"/ENTRY:TKWinMain"
    })
    includedirs("C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Tools/MSVC/14.30.30705/crt/src/vcruntime")
    includedirs({
        ".",
        "../",
        "../../",
        "../components/",
        "../../components"
    })
    include_skia()