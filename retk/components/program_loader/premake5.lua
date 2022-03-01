-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

component("program_loader")
    files({
        "**.cc",
        "**.h",
        "**.in"
    })
    dependencies("fmtlib")
    -- header only:
    includedirs({
        blu.extdir .. "/rapidjson/include",
    })