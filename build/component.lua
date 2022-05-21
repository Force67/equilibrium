-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.
-- This implements the component model used for the buildsystem.

scope_name = "Components"

function push_scope(name)
    scope_name = name
end

function scope_group(name)
    group(scope_name .. "/" .. name)
end

-- Declare a component; use this in place of 'project'
function component(name)
    group(scope_name .. "/" .. name)
    project(name)
        kind("StaticLib")
        includedirs({
            ".",        --< current dir
            "../",      --< component dir
            blu.rootdir,--< main dir for direct base access
        })
        -- each component links against google mock
        dependencies("googlemock")
        links("base")
end

-- i could put in the effort to get this working nicely, 
-- but it gives us some development comfort to be able to edit those for test files
function strip_testfiles()
    removefiles("**_test.cc")
end

-- there may be several tests for one type of component
-- the name follows the following scheme component_name:test_name
function unittest(name)
    -- convert seperator into path
    group(scope_name .. "/" .. string.gsub(name, ":", "/"))
    local pname = string.match(name, ":(.*)") or name
    project(pname)
        kind("ConsoleApp")
        includedirs({
            ".",
            "../../",
            blu.rootdir
        })
        dependencies("googlemock")
        links("base")
end