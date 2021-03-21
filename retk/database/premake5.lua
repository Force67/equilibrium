-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

project("database")
    kind("StaticLib")
    files({
        "*.cc",
        "*.h",
    })
    links({
        "sqlite"
    })
    includedirs({
        ".",
        blu.extdir .. "/sqlite",
        blu.extdir .. "/fmt/include",
    })

group("Tests")
project("database_tests")
    kind("ConsoleApp")
    files({
        "test/main.cpp"
    })
    links({
        "database"
    })
    includedirs({
        ".",
        blu.extdir .. "/sqlite",
        blu.extdir .. "/fmt/include",
    })