-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

component("diff")
    files({
        "*.cc",
        "*.h",
    })
    dependencies("fmtlib")
    -- header only:
    includedirs({
        blu.extdir .. "/rapidjson/include",
    })

unittest("diff:multimatch")
    include_meta()
    files({
        "test/test_runner.cc",
        "*.cc",
        "*.h"
    })
    links({"base", "googlemock"})
    includedirs({
        ".",
        "../",
        "../../",
        blu.extdir .. "/rapidjson/include",
        blu.extdir .. "/fmt/include",
        blu.extdir .. "/googletest/googletest/include",
    })