-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

project("runner")
    kind("ConsoleApp")
    targetname("nemisis_cli")
    include_meta()
    files({
        "main.cc"
    })
    includedirs({"."})