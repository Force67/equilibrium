-- Copyright (c) NOMAD Group<nomad-group.net>

function network_include()
    links({
        "sockpp",
        "fmtlib",
        "utility",
    })
    includedirs({
        ".",
        "../",
        blu.extdir .. "/xenium",
        blu.extdir .. "/sockpp/include",
        blu.extdir .. "/flatbuffers/include",
        blu.extdir .. "/fmt/include",
    })
end

project("network")
    kind("StaticLib")
    files({
        "*.cpp",
        "*.h",
    })
    network_include()

project("testclient")
    kind("ConsoleApp")
    include_meta()
    files({
        "tests/test_client/Main.cpp",
    })
    network_include()
    links({
        "fmtlib",
        "network"
    })
    includedirs({
        "tests/test_client"
    })

project("testserver")
    kind("ConsoleApp")
    include_meta()
    files({
        "tests/test_server/Main.cpp",
    })
    network_include()
    links({
        "fmtlib",
        "network"
    })
    includedirs({
        "tests/test_server"
    })