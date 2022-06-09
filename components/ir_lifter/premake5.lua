-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

component("ir_lifter")
    files({
        "**.cc",
        "**.cpp",
        "**.h",
        "**.inl"
    })
    --include_llvm()
    dependencies({
        "fmtlib",
        "capstone",
        "zydis"
    })
    -- header only:
    includedirs({
        blu.extdir .. "/rapidjson/include",
    })