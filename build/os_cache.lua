local disabled = os.getenv("PREMAKE_FILE_CACHE_DISABLE") == "1"

if disabled then
  return
end

local function shallow_copy(list)
  if type(list) ~= "table" then
    return list
  end

  local copy = {}
  for i = 1, #list do
    copy[i] = list[i]
  end
  return copy
end

local function make_key(...)
  local parts = {}
  for i = 1, select("#", ...) do
    local value = select(i, ...)
    parts[i] = tostring(value)
  end
  return table.concat(parts, string.char(31))
end

local matchfiles_cache = {}
local matchdirs_cache = {}

local original_matchfiles = os.matchfiles
os.matchfiles = function(...)
  local key = make_key(...)
  local cached = matchfiles_cache[key]
  if cached then
    return shallow_copy(cached)
  end

  local result = original_matchfiles(...)
  matchfiles_cache[key] = result
  return shallow_copy(result)
end

local original_matchdirs = os.matchdirs
os.matchdirs = function(...)
  local key = make_key(...)
  local cached = matchdirs_cache[key]
  if cached then
    return shallow_copy(cached)
  end

  local result = original_matchdirs(...)
  matchdirs_cache[key] = result
  return shallow_copy(result)
end

return {
  stats = function()
    return {
      files = matchfiles_cache,
      dirs = matchdirs_cache,
    }
  end
}
