
project "CWCore"
    language "C++"
    kind "SharedLib"

    --vpaths { ["*"] = "*" }

    includedirs {
        ".",
        "../shared",
    
    }

	defines {
		'COMPILING_CORE'
	}
	
	links {
        "shared",
		"capstone"
    }

    files {
        "premake5.lua",
        "**.h",
        "**.hpp",
        "**.cpp",
		"*.def",
    }