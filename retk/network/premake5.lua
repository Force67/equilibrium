-- Copyright (c) NOMAD Group<nomad-group.net>

function network_include()
    links({
        "sockpp",
        "fmtlib",
        "base",
    })
    includedirs({
        ".",
        "../",
        "../../",
        blu.extdir .. "/xenium",
        blu.extdir .. "/sockpp/include",
        blu.extdir .. "/flatbuffers/include",
        blu.extdir .. "/fmt/include",
    })
end

group("retk/network")
project("network")
    kind("StaticLib")
    files({
        "base/*.cc",
        "base/*.h",
        "tksp/*.h",
        "tksp/*.cc",
        "util/*.cc",
        "util/*.h"
    })
    network_include()

group("retk/network/test")
project("testclient")
    kind("ConsoleApp")
    include_meta()
    files({
        "test/client/main.cc",
    })
    network_include()
    links({
        "fmtlib",
        "network"
    })
    includedirs({
        "test/client"
    })

group("retk/network/test")
project("testserver")
    kind("ConsoleApp")
    include_meta()
    files({
        "test/server/main.cc",
    })
    network_include()
    links({
        "fmtlib",
        "network"
    })
    includedirs({
        "test/server"
    })