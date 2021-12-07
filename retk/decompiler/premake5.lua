-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

component("decompiler")
    files({
        "**.cc",
        "**.h",
        "**.inl"
    })
    dependencies({
        "fmtlib",
        "capstone"
    })
    -- header only:
    includedirs({
        blu.extdir .. "/rapidjson/include",
    })