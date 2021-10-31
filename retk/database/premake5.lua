-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

component("database")
    files({
        "sqlite/*.cc",
        "sqlite/*.h",
    })
    dependencies({
        "sqlite",
        "fmtlib"
    })
    includedirs({
        blu.extdir .. "/googletest/googletest/include",
    })

unittest("database:sqlite_test")
    files({
        "sqlite/test/run_all_tests.cc",
        "sqlite/test/sqlite_test_suite.cc",
        "sqlite/test/sqlite_test_suite.h",
        -- for now, just to get tests working...
        "sqlite/*.cc",
        "sqlite/*.h",
    })
    dependencies({"sqlite", "fmtlib"})