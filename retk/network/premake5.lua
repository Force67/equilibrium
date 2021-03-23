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

project("network")
    kind("StaticLib")
    files({
        "core/*.cc",
        "core/*.h",
        "tcp_stack/*.cc",
        "tcp_stack/*.h"
    })
    network_include()

project("testclient")
    kind("ConsoleApp")
    include_meta()
    files({
        "tests/client/main.cc",
    })
    network_include()
    links({
        "fmtlib",
        "network"
    })
    includedirs({
        "tests/client"
    })

project("testserver")
    kind("ConsoleApp")
    include_meta()
    files({
        "tests/server/main.cc",
    })
    network_include()
    links({
        "fmtlib",
        "network"
    })
    includedirs({
        "tests/server"
    })