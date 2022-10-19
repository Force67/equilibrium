-- Copyright (C) 2021 Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

function eq_network_include()
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

component2("eq_network")
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
    eq_network_include()

unittest2("eq_network:unittests")
    include_meta()
    files({
        "test/run_all_unittests.cc",
    })
    eq_network_include()
    links({
        "eq_network",
    })
    includedirs({
        "./test",
    })

unittest2("eq_network:testclient")
    include_meta()
    files({
        "test/client/main.cc",
    })
    eq_network_include()
    links({
        "eq_network"
    })
    includedirs({
        "test/client"
    })

unittest2("eq_network:testserver")
    include_meta()
    files({
        "test/server/main.cc",
    })
    eq_network_include()
    links({
        "eq_network"
    })
    includedirs({
        "test/server"
    })