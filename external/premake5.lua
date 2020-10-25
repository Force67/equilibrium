-- Copyright (c) NOMAD Group<nomad-group.net>

require("premake-idaqt/qt")
qt = premake.modules.qt

function enet_include()
    includedirs({
        "enet/include"
    })
    files({
        "enet/include/*.h",
        "enet/*.c"
    })
	removefiles({
		"enet/unix.c",
		"include/enet/unix.h"
    })
    filter("system:windows")
        links({
            "Ws2_32", 
            "winmm"
        })
end

-- enet built with extra functions for
-- seemless interop
local p = project("ENetNative")
    kind("SharedLib")
    language("C")
    defines({
        "ENET_DLL",
        "ENET_BUILDING_LIB",
        "ENET_BUILD_MANAGED"
    })
    enet_include()

project("enet")
    kind("StaticLib")
    language("C")
    enet_include()

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

project("GoogleMock")
    language("C++")
    kind("StaticLib")
    includedirs({
        "googletest/googlemock/include",
        "googletest/googletest/include",
        "googletest/googlemock",
        --"googletest/googletest"
    })
    files({
        "googletest/googlemock/src/gmock-all.cc",
        --"googletest/googlemock/src/*.cc",
        --"googletest/googletest/src/*.cc",
        "googletest/googlemock/include/gmock/**.h",
        --"googletest/googletest/include/gtes/**.h",
    })
