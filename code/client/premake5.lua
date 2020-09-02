-- Copyright (c) NOMAD Group<nomad-group.net>

project("NODA - client")
    kind("SharedLib")
    targetname("Noda_72")
    files({
        "**.cpp",
        "**.h",
    })
    links({
        -- ida libraries
        "ida",
        "pro",
        "compress"
    })
    defines("__NT__")
    includedirs({
        ".",
        blu.extdir .. "/idasdk72/include"
    })
    libdirs({
        blu.extdir .. "/idasdk72/lib/x64_win_vc_64"
    })