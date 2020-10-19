-- Copyright (c) NOMAD Group<nomad-group.net>

project("DedicatedMain")
    kind("ConsoleApp")
    include_meta()
    files({
        "**.cpp",
        "**.h",
        "Noda_Server.rc",
    })
    links({
        "server",
        "netlib"
    })
    dependson("server")
    includedirs({
        ".",
        "../../common",
        "../server",
        blu.extdir .. "/enet/include"
    })