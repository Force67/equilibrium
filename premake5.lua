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
    optimize("Full")
    flags({
        "LinkTimeOptimization"
    })
    defines("SHIPPING")

filter("language:C or C++")
    vectorextensions("SSE4.2")
    staticruntime("on")

filter("language:C++")
    cppdialect("C++20")

filter("system:windows")
    defines("OS_WIN")
filter{}

-- This is here rather temporarily
defines("PROJECT_NAME=\"%{prj.name}\"")    

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
    -- external must always come first
    -- so 'dependencies' can link against them
    include("./external")
    include("./base")
    include("./retk")
    include("./apps")
    include("./tools")

    group("Plugins")
    include("plugins/ida")
    include("plugins/x64dbg")

    group("Server")
    include("sync_server/dedi_server")
    include("sync_server/server")