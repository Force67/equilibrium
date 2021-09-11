-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

include("./build")

local function insertASAN(prj)
	premake.w('<EnableASAN>true</EnableASAN>')
end

-- todo: move this
premake.override(premake.vstudio.vc2010.elements, "configurationProperties", function(base, cfg)
	local calls = base(cfg)

    if premake.vstudio.projectPlatform(cfg) == "DebugAsan" then
        table.insertafter(calls, premake.vstudio.vc2010.useDebugLibraries, insertASAN)
    end

	return calls
end)

staticruntime("on")

filter("architecture:x86_64")
    targetsuffix("_64")

filter("configurations:Debug")
    defines("TK_DBG")

filter("configurations:DebugAsan")
    defines("TK_DBG")
    flags("NoIncrementalLink")

filter("configurations:Release")
    runtime("Release")
    optimize("Speed")

filter("configurations:Shipping")
    runtime("Release")
    optimize("Speed")
    flags({
        "LinkTimeOptimization"
    })

filter("language:C or C++")
    vectorextensions("SSE4.1")

filter("language:C++")
    cppdialect("C++20")

filter("system:windows")
    defines("OS_WIN")
    
workspace("RETK")
    configurations({
        "Debug",
        "DebugAsan",
        "Release",
        "Shipping",
    })
    flags {
        "MultiProcessorCompile"
    }
    defines("NOMINMAX")

    include("./apps")
    include("./external")
    include("./base")
    include("./retk")

    group("Plugins")
    include("plugins/ida_plugin")
    include("plugins/x64dbg_plugin")

    group("Server")
    include("sync_server/dedi_server")
    include("sync_server/server")