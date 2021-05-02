-- Copyright (C) 2021 Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

group("Apps")
project("App_Main")
    kind("WindowedApp")
    targetname("ReTK")
    include_meta()
    files({
        "retk_full/retk_full.rc",
        "retk_full/**.cc",
        "retk_full/**.h",
        "premake5.lua"
    })
    links({
        -- third party
        "fmtlib",
        "skia",

        -- core libraries
        "network",
        "database",
        "base",
        "sync",
    })
    linkoptions({
        -- use our regular main instead of winmain
        "/ENTRY:mainCRTStartup"
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
    })
    libdirs({
        blu.extdir .. "/skia_sdk/lib/debug_static",
    })