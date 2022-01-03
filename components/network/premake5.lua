-- Copyright (C) 2021 Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

function network_include()
    dependencies({
        "sockpp",
        "fmtlib",
        "googlemock",
    })
    -- header only:
    includedirs({
        blu.extdir .. "/xenium",
        blu.extdir .. "/flatbuffers/include",
    })
end

component("network")
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

unittest("network:unittests")
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

unittest("network:testclient")
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

unittest("network:testserver")
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