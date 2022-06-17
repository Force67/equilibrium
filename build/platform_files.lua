-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.
local system_ext_map = {windows = "win", linux = "linux", linux = "posix", macosx = "mac"}

local first = true
local rval = "files:"
for key, value in pairs(system_ext_map) do
  if key ~= _TARGET_OS then
    local sourceglob = string.format("**%s.cc or **/%s/**", value, value)
    rval = rval .. (first and "" or " or ") .. sourceglob
    if first then first = false end
  end
end

filter(rval)
flags("ExcludeFromBuild")
filter {}
