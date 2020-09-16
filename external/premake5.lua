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
    targetdir(blu.bindir .. "/netcoreapp3.1")
    enet_include()

project("enet")
    kind("StaticLib")
    language("C")
    enet_include()

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

project("Flatbuffers")
    language("C#")
    kind("SharedLib")
    location(blu.netout)
    objdir(blu.netdir)
    dotnetframework("netcoreapp3.1")
    includedirs({
        "flatbuffers/net"
    })
    files({
        "flatbuffers/net/Flatbuffers/*.cs"
    })