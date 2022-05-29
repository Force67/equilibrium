-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

component("program_database")
    -- header only:
    includedirs({
        "../",
        blu.extdir
    })
    files({
        "**.cc",
        "**.h",
        "**.in"
    })
    dependencies(
        "program_loader",
        "fmtlib")

unittest("program_database:database_test")
includedirs({
    "../",
    blu.extdir,
    blu.extdir .. "/fmt/include",
})
    add_generic_test_main()
    files({
        "**.cc",
        "**.h",
        "**.in"
    })
    dependencies(
        "program_loader",
        "fmtlib")