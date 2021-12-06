-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

component("entitlement")
    files({
        "*.cc",
        "*.h",
    })
    dependencies("fmtlib")
    -- header only:
    includedirs({
        blu.extdir .. "/rapidjson/include",
    })