-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.
-- This implements the component model used for the buildsystem.

-- Declare a component; use this in place of 'project'
function component(name)
    group("Components/" .. name)
    project(name)
        kind("StaticLib")
        includedirs({
            ".",        --< current dir
            "../",      --< component dir
            "../../",   --< main dir for direct base access
        })
        -- each component links against google mock
        dependencies("googlemock")
        links("base")
end

-- there may be several tests for one type of component
function unittest(name, options)
    group("Components/" .. name)
    project(name)
        kind("ConsoleApp")
        dependencies("googlemock")
end