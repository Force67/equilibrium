-- Copyright (C) 2021 Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

group("Tools")
project("Hits")
    kind("ConsoleApp")
    include_meta()
    files({
        "hits/**.cc",
        "hits/**.h"
    })
    links({
        "base",
    })
    dependencies({
        "fmtlib",
    })
    includedirs({
        ".",
        "../",
        "../retk",
        blu.extdir .. "/mem/include",
    })