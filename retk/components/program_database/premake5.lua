-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

component("program_database")
    files({
        "**.cc",
        "**.h",
        "**.in"
    })
    dependencies(
        "program_loader",
        "fmtlib")
    -- header only:
    includedirs({
        blu.extdir .. "/rapidjson/include",
    })

unittest("program_database:test")
    add_generic_test_main()
    files({
        "**.cc",
        "**.h",
        "**.in"
    })
    dependencies(
        "program_loader",
        "fmtlib")
    -- header only:
    includedirs({
        blu.extdir .. "/rapidjson/include",
    })