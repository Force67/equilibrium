-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

project("dedi_server")
    kind("ConsoleApp")
    targetname("retksrv")
    include_meta()
    files({
        "main.cc",
        "dedi_server.rc",
    })
    links({
        "server",
        "network",
        "fmtlib",
        "sync"
    })
    dependson("server")
    includedirs({
        ".",
        "../../common",
        "../server",
        blu.extdir .. "/fmt/include",
        blu.extdir .. "/sockpp/include"
    })