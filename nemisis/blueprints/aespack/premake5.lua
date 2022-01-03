
project "Blueprint-Aes"
    language "C++"
    kind "ConsoleApp"
	flags { "Maps", "NoBufferSecurityCheck" }
	optimize "Speed"
	objdir "%{cfg.targetdir}"

    includedirs {
        ".",
        "../shared",
    
    }

	links {
        "shared",
    }

    files {
        "premake5.lua",
        "**.h",
        "**.hpp",
        "**.cpp",
		"**.c",
    }