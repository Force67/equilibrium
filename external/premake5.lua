-- Copyright (c) NOMAD Group<nomad-group.net>

require("premake-idaqt/qt")
qt = premake.modules.qt

project("sockpp")
    kind("StaticLib")
    language("C++")
    includedirs({
        "sockpp/include"
    })
    files({
        "sockpp/include/sockpp/*.h",
        "sockpp/src/*.cpp"
    })
    filter("system:windows")
        links("Ws2_32")
        removefiles("sockpp/include/sockpp/unix_*")

project("sqlite")
    kind("StaticLib")
    language("C")
    includedirs({
        "./sqlite"
    })
    files({
        "sqlite/*.h",
        "sqlite/*.c"
    })
    defines({
        "SQLITE_ENABLE_SQLLOG",
    })

    filter ("configurations:Debug")
        defines("SQLITE_DEBUG")

project("fmtlib")
    language("C++")
    kind("StaticLib")
    includedirs({
        "fmt/include",
    })
    files({
        "fmt/src/format.cc",
        "fmt/src/os.cc",
    })

project("googlemock")
    language("C++")
    kind("StaticLib")
    includedirs({
        "googletest/googlemock/include",
        "googletest/googletest/include",
        "googletest/googlemock",
        "googletest/googletest"
    })
    files({
        "googletest/googlemock/src/gmock-all.cc",
        "googletest/googletest/src/gtest-all.cc",
        --"googletest/googlemock/src/*.cc",
        --"googletest/googletest/src/*.cc",
        "googletest/googlemock/include/gmock/**.h",
        --"googletest/googletest/include/gtes/**.h",
    })
