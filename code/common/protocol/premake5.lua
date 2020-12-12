-- Copyright (c) NOMAD Group<nomad-group.net>

project("protocol")
    -- excludes the project from build
    kind("None")
    files({
        "**.fbs"
    })