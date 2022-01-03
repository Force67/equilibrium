
project "CWFrontend"
    language "C++"
    kind "ConsoleApp"
    targetname "cwf"
	rtti "Off"

    --vpaths { ["*"] = "*" }

    includedirs {
        ".",
		"../cwcore",
        "../shared",
    }
	
	links {
        "shared",
		"cwcore"
    }

    files {
        "premake5.lua",
        "**.h",
        "**.hpp",
        "**.cpp",
    }