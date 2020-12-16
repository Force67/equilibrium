-- Copyright (c) NOMAD Group<nomad-group.net>

project("dedi_server")
    kind("ConsoleApp")
    targetname("retksrv")
    include_meta()
    files({
        "**.cpp",
        "**.h",
        "dedi_server.rc",
    })
    links({
        "server",
        "network",
        "fmtlib"
    })
    dependson("server")
    includedirs({
        ".",
        "../../common",
        "../server",
        blu.extdir .. "/fmt/include",
        blu.extdir .. "/sockpp/include"
    })