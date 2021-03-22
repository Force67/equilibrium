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
        "*.cc",
        "*.h",
    })
    network_include()

project("testclient")
    kind("ConsoleApp")
    include_meta()
    files({
        "tests/client/Main.cpp",
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
        "tests/server/Main.cpp",
    })
    network_include()
    links({
        "fmtlib",
        "network"
    })
    includedirs({
        "tests/server"
    })