
project "DummyApp"
    language "C++"
    kind "ConsoleApp"
	flags "Maps" -- Enable Map File generation for codewrangler

    includedirs {
        ".",
		"../../sdk",
    }
	
	links {
		"cwsdk"
	}
	
    files {
        "premake5.lua",
        "**.h",
        "**.hpp",
        "**.cpp",
    }