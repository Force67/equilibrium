-- Copyright (C) 2022 Vincent Hengel.
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