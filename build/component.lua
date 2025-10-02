-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

scope_name = "Components"

local function profile_scope(label, fn)
  if blu and blu.profile and blu.profile.scope then
    return blu.profile.scope(label, fn)
  end
  return fn()
end

function push_scope(name)
  scope_name = name
end

function scope_group(name)
  group(scope_name .. "/" .. name)
end

-- use this instead of scope groups.
function grouped_include(name, namespace)
  profile_scope("grouped_include:" .. tostring(name), function()
    group(namespace)
    include(name)
    group("")
  end)
end

-- Declare a component; use this in place of 'project'
function component(name)
  local start, final, match = name:find(":(.*)")
  local p_name = match or name

  if match ~= nil then
    --name = string.gsub(name, ":", "/")
    name = string.sub(name, 1, start - 1)
  end

  profile_scope("component:" .. tostring(p_name), function()
    group(scope_name .. "/" .. name)
      project(p_name)
        kind("StaticLib")
        includedirs({
          ".",        --< current dir
          "../",      --< component dir
          blu.rootdir,--< main dir for direct base access
        })
        -- each component links against google mock
        dependencies("googlemock")
        links("base")
  end)
end

function component2(name)
  local start, final, match = name:find(":(.*)")
  local p_name = match or name

    profile_scope("component2:" .. tostring(p_name), function()
      project(p_name)
        kind("StaticLib")
        includedirs({
          ".",        --< current dir
          "../",      --< component dir
          blu.rootdir,--< main dir for direct base access
        })
        -- each component links against google mock
        dependencies("googlemock")
        links("base")
    end)
end

function shared_component2(name)
  local start, final, match = name:find(":(.*)")
  local p_name = match or name

    profile_scope("shared_component2:" .. tostring(p_name), function()
      project(p_name)
        kind("SharedLib")
        includedirs({
          ".",        --< current dir
          "../",      --< component dir
          blu.rootdir,--< main dir for direct base access
        })
        -- each component links against google mock
        dependencies("googlemock")
        links("base_shared")
    end)
end
