-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

group("Components/database")
project("database")
    kind("StaticLib")
    files({
        "sqlite/*.cc",
        "sqlite/*.h",
    })
    links({
        "sqlite",
        "base",
        "googlemock"
    })
    includedirs({
        ".",
        "../../",
        blu.extdir .. "/sqlite",
        blu.extdir .. "/fmt/include",
        blu.extdir .. "/googletest/googletest/include",
    })

group("Components/database/test")
project("sqlite_database_test")
    kind("ConsoleApp")
    files({
        "sqlite/test/run_all_tests.cc",
        "sqlite/test/sqlite_test_suite.cc",
        "sqlite/test/sqlite_test_suite.h",
        -- for now, just to get tests working...
        "sqlite/*.cc",
        "sqlite/*.h",

    })
    links({
        --"database",
        "base",
        "googlemock",
        "base",

        -- for now, just to get tests working...
        "sqlite",
    })
    includedirs({
        ".",
        "../../",
        blu.extdir .. "/sqlite",
        blu.extdir .. "/fmt/include",
        blu.extdir .. "/googletest/googletest/include",
    })