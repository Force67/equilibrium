-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

group("retk/database")
project("database")
    kind("StaticLib")
    files({
        "sqlite/*.cc",
        "sqlite/*.h",
    })
    links({
        "sqlite",
        "googlemock"
    })
    includedirs({
        ".",
        blu.extdir .. "/sqlite",
        blu.extdir .. "/fmt/include",
        blu.extdir .. "/googletest/googletest/include",
    })

group("retk/database/test")
project("sqlite_database_test")
    kind("ConsoleApp")
    files({
        "sqlite/test/run_all_tests.cc",

        -- for now, just to get tests working...
        "sqlite/*.cc",
        "sqlite/*.h",

    })
    links({
        --"database",
        "googlemock",

        -- for now, just to get tests working...
        "sqlite",
    })
    includedirs({
        ".",
        blu.extdir .. "/sqlite",
        blu.extdir .. "/fmt/include",
        blu.extdir .. "/googletest/googletest/include",
    })