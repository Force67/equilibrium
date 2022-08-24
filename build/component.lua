-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

scope_name = "Components"

function push_scope(name)
  scope_name = name
end

function scope_group(name)
  group(scope_name .. "/" .. name)
end

-- Declare a component; use this in place of 'project'
function component(name)
  local start, final, match = name:find(":(.*)")
  local p_name = match or name

  if match ~= nil then
    --name = string.gsub(name, ":", "/")
    name = string.sub(name, 1, start - 1)
  end

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
end