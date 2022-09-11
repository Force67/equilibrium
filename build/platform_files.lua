-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.
local system_ext_map = {windows = {"win"}, linux = {"linux", "posix"},  macosx = {"mac", "posix"}}	-- map of system to extension

local first = true
local rval = "files:"
for key, value_map in pairs(system_ext_map) do
  for i, value in pairs(value_map) do
    if key ~= _TARGET_OS then
      -- ignore duplicate entries
      if string.find(rval, value) == nil and system_ext_map[key].valuu ~= system_ext_map[_TARGET_OS].value then
        local sourceglob = string.format("**_%s.cc or **/%s/**", value, value)
        rval = rval .. (first and "" or " or ") .. sourceglob
        if first then first = false end
      end
    end
  end
end

print(rval)

filter(rval)
flags("ExcludeFromBuild")
filter {}

function opt_include(path)
  if os.isdir(path) then
    include(path)
  end
end