-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.
local p = premake

p.family = p.api.container("family", p.workspace)
p.family.placeholder = true

function p.family.new(name)
  return p.container.new(p.family, name)
end

-- override the project instantiation with this creation fn
-- this is implemented exactly how premake implements it's "group" function
-- see https://github.com/premake/premake-core/blob/master/src/base/group.lua
-- for reference
premake.override(premake.project, "new", function(oldfn, name)
  local prj = oldfn(name)

  -- register our own property
  if p.api.scope.family then
    prj.family = p.api.scope.family.name
  else
    prj.family = ""
  end

  return prj
end)
  