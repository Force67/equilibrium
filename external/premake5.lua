-- Copyright (c) NOMAD Group<nomad-group.net>

require("premake-idaqt/qt")
qt = premake.modules.qt

function declare_enet(standAlone)
    if standAlone then
        project("enet_sv")
        kind("SharedLib")
        defines({
            "ENET_DLL",
            "ENET_BUILDING_LIB"
        })
    else
        project("enet")
        kind("StaticLib")
    end
    language("C")
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

declare_enet(true)
declare_enet(false)