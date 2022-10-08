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
-- we can be up to 3 levels deep, as in
-- <component_name>:<subproject>:<test_name>
local function tokenize(inputstr, sep)
  local t={}
  for str in string.gmatch(inputstr, "([^:]+)") do
    table.insert(t, str)
  end
  return t
end

function unittest(name)
  local tokens = tokenize(name)
  if #tokens == 0 then
    error("Unittest name is empty")
  end

  local component_name = nil
  local project_name = nil
  for i = 1, #tokens do
    --print(i, ". ", tokens[i])
    if i == 1 then
      component_name = tokens[i]
    end

    if i == #tokens then
      project_name = tokens[i]
    end
  end

  if component_name == nil or project_name == nil then
    error("Failed to find a component or subproject name")
  end
  
  group(scope_name .. "/" .. component_name .. "/tests")
    project(project_name)
      kind("ConsoleApp")
      add_generic_test_main()
      includedirs({
        ".",        --< current dir
        "../",      --< component dir
        blu.rootdir,--< main dir for direct base access
      })
      dependencies("googlemock")
      links("base")
end

function unittest2(name)
  local tokens = tokenize(name)
  if #tokens == 0 then
    error("Unittest name is empty")
  end

  local component_name = nil
  local project_name = nil
  for i = 1, #tokens do
    --print(i, ". ", tokens[i])
    if i == 1 then
      component_name = tokens[i]
    end

    if i == #tokens then
      project_name = tokens[i]
    end
  end

  if component_name == nil or project_name == nil then
    error("Failed to find a component or subproject name")
  end
  
  --group(scope_name .. "/" .. component_name .. "/tests")
    project(project_name)
      kind("ConsoleApp")
      add_generic_test_main()
      includedirs({
        ".",        --< current dir
        "../",      --< component dir
        blu.rootdir,--< main dir for direct base access
      })
      dependencies("googlemock")
      links("base")
end