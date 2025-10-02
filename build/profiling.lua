local M = {}

local table_unpack = table.unpack or unpack

local function now()
  return os.clock()
end

local function format_seconds(seconds)
  local total_ms = math.floor(seconds * 1000 + 0.5)
  local s = math.floor(total_ms / 1000)
  local ms = total_ms % 1000
  return string.format("%d.%03ds", s, ms)
end

local function read_threshold()
  local env = os.getenv("PREMAKE_PROFILE_THRESHOLD_MS")
  local value = tonumber(env or "")
  if value and value >= 0 then
    return value / 1000.0
  end
  return 0.005
end

M.threshold = read_threshold()
M.depth = 0

local function log_event(kind, label, elapsed, depth)
  if elapsed < M.threshold then
    return
  end

  local indent = string.rep("  ", math.max(depth - 1, 0))
  local message = string.format("[premake] %s%s %s", indent, kind, format_seconds(elapsed))

  if label and #label > 0 then
    message = string.format("%s :: %s", message, label)
  end

  io.stderr:write(message .. "\n")
end

local function call_with_timing(kind, label, fn, args)
  M.depth = M.depth + 1
  local depth = M.depth
  local start_ts = now()
  local results = table.pack(fn(table_unpack(args, 1, args.n or #args)))
  local elapsed = now() - start_ts
  M.depth = M.depth - 1
  log_event(kind, label, elapsed, depth)
  return table_unpack(results, 1, results.n)
end

function M.scope(label, fn, ...)
  local args = table.pack(...)
  return call_with_timing("scope", label or "", function(...)
    return fn(...)
  end, args)
end

function M.install()
  local original_include = include
  include = function(script, ...)
    local args = table.pack(script, ...)
    return call_with_timing("include", tostring(script), function(...)
      return original_include(...)
    end, args)
  end

  local original_require = require
  require = function(modname, ...)
    local args = table.pack(modname, ...)
    return call_with_timing("require", tostring(modname), function(...)
      return original_require(...)
    end, args)
  end

  if premake and premake.generate then
    local original_generate = premake.generate
    premake.generate = function(obj, filename, callback)
      local args = table.pack(obj, filename, callback)
      return call_with_timing("generate", tostring(filename), function(...)
        return original_generate(...)
      end, args)
    end
  end
end

return M
