-- Copyright (C) 2021 Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

function network_include()
    links({
        "sockpp",
        "fmtlib",
        "googlemock",
        "base"
    })
    includedirs({
        ".",
        "../",
        "../../",
        blu.extdir .. "/xenium",
        blu.extdir .. "/sockpp/include",
        blu.extdir .. "/flatbuffers/include",
        blu.extdir .. "/fmt/include",
        blu.extdir .. "/googletest/googletest/include",
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
        "zeta/*.h",
        "zeta/*.cc",
        "util/*.cc",
        "util/*.h",

        -- documentation
        "zeta/readme.md",
    })
    network_include()

group("retk/network/test")
project("network_unittests")
    kind("ConsoleApp")
    include_meta()
    files({
        "test/run_all_unittests.cc",
    })
    network_include()
    links({
        "network",
    })
    includedirs({
        "./test",
    })

group("retk/network/test")
project("testclient")
    kind("ConsoleApp")
    include_meta()
    files({
        "test/client/main.cc",
    })
    network_include()
    links({
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
        "network"
    })
    includedirs({
        "test/server"
    })