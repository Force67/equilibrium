-- Copyright (C) 2021 Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

project("protocol")
    -- excludes the project from build
    kind("None")
    files({
        "**.fbs"
    })