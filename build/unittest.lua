-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

-- i could put in the effort to get this working nicely, 
-- but it gives us some development comfort to be able to edit those for test files
function strip_testfiles()
    removefiles("**_test.cc")
end

function add_generic_test_main()
    files(blu.rootdir .. "/build/generic_test_main.cc")
end

-- there may be several tests for one type of component
-- the name follows the following scheme component_name:test_name
function unittest(name)
    local project_name = string.match(name, ":(.*)") or name
    -- convert seperator into path
    name = string.gsub(name, ":", "/")
    group(scope_name .. "/" .. string.sub(name, 0, string.find(name, "/[^/]*$")) .. "tests")
    project(project_name)
        kind("ConsoleApp")
        includedirs({
            ".",
            "../../",
            blu.rootdir
        })
        dependencies("googlemock")
        links("base")
end