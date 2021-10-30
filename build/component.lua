-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.
-- This implements the component model used for the buildsystem.

-- Declare a component; use this in place of 'project'
function component(name)
    group("Components/" .. name)
        project(name)
        kind("StaticLib")
        includedirs({
            ".",
            "../../",
        })
        -- each component links against google mock
        links("base", "googlemock")
end

function unittest(name, options)
    project(name)
    kind("ConsoleApp")
    links("googlemock")
    includedirs(blu.extdir .. "/googletest/googletest/include")
end