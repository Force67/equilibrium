-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.
-- maps the system to extensions
local system_extension_map = {
  windows = {"win"}, 
  linux = {"linux", "posix"},
  macosx = {"mac", "posix"}
}

local function intersects_target(target_value)
  for index, value in pairs(system_extension_map[_TARGET_OS]) do
    if value == target_value then
      return true
    end
  end
  return false
end

local first = true
local rval = "files:"

for key, value_map in pairs(system_extension_map) do
  for index, value in pairs(value_map) do
    if key ~= _TARGET_OS and value then
      -- ignore duplicate entries e.g 2x posix
      if string.find(rval, value) == nil and not intersects_target(value) then
        rval = rval .. (first and "" or " or ") .. string.format("**_%s.cc or **/%s/**", value, value)
        if first then first = false end
      end
    end
  end
end

-- install the global filter for those files.
filter(rval)
  flags("ExcludeFromBuild")
filter {}