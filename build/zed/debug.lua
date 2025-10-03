local p = premake

p.modules.zed_debug = {}
local m = p.modules.zed_debug

local workspace = p.workspace
local project = p.project

local function split_configs(value)
  if not value or value == "" then
    return { debug = true, release = true }
  end

  value = value:lower()
  if value == "all" then
    return nil
  end

  local set = {}
  for token in value:gmatch("[^,%s]+") do
    set[token] = true
  end
  return set
end

local function json_escape(str)
  str = str:gsub('\\', '\\\\')
  str = str:gsub('"', '\\"')
  str = str:gsub('\n', '\\n')
  str = str:gsub('\r', '\\r')
  return str
end

local function is_debuggable(prj)
  return prj.kind == "ConsoleApp" or prj.kind == "WindowedApp"
end

local function should_include(cfg, allowed)
  if not allowed then
    return true
  end
  return allowed[cfg.shortname] == true
end

local function escape_pattern(value)
  return value:gsub('([^%w])', '%%%1')
end

local mock_cache = {}
local relative_cache = {}
local last_fake_action = nil
local last_action = nil
local mock_pattern = nil
local mock_replacement = nil
local fallback_pattern = nil
local fallback_replacement = nil

local function update_mock_cache()
  local fake = _OPTIONS["zed-fakeaction"] or "gmake2"
  if fake ~= last_fake_action or _ACTION ~= last_action then
    last_fake_action = fake
    last_action = _ACTION
    mock_cache = {}
    relative_cache = {}
    local escaped_action = escape_pattern(_ACTION)
    mock_pattern = string.format("/%s/", escaped_action)
    mock_replacement = string.format("/%s/", fake)
    fallback_pattern = escaped_action
    fallback_replacement = fake
  end
end

local function mock_action_path(value)
  update_mock_cache()
  local cached = mock_cache[value]
  if cached then
    return cached
  end

  local result, count = value:gsub(mock_pattern, mock_replacement)
  if count == 0 then
    result = result:gsub(fallback_pattern, fallback_replacement)
  end

  mock_cache[value] = result
  return result
end

local function get_family_dir(prj)
  local family = prj.family or ""
  if family == "" then
    return nil
  end

  local dir = path.join(blu.rootdir, "projects", family)
  if os.isdir(dir) then
    return dir
  end

  return nil
end

local function has_family_script(family_dir)
  if not family_dir then
    return false
  end

  return os.isfile(path.join(family_dir, "make.sh"))
end

local family_script_cache = {}

local function get_family_script_command(family_dir)
  if not family_dir then
    return nil
  end

  local cached = family_script_cache[family_dir]
  if cached ~= nil then
    return cached or nil
  end

  local command
  if has_family_script(family_dir) then
    local rel_dir = path.getrelative(blu.rootdir, family_dir)
    command = string.format("./%s/make.sh", rel_dir)
  end

  family_script_cache[family_dir] = command or false
  return command
end

local function build_command(prj_name, cfg_shortname, family_script)
  local make_cmd = string.format("make -C out/gmake2 %s config=%s", prj_name, cfg_shortname)
  if family_script then
    return string.format("cd \"$ZED_WORKTREE_ROOT\" && %s %s && %s", family_script, cfg_shortname, make_cmd)
  end

  return string.format("cd \"$ZED_WORKTREE_ROOT\" && %s", make_cmd)
end

local function relative_to_root(pathname)
  local cached = relative_cache[pathname]
  if cached then
    return cached
  end

  local rel = path.translate(path.getrelative(blu.rootdir, pathname))
  rel = mock_action_path(rel)
  local result
  if rel == "." then
    result = "$ZED_WORKTREE_ROOT"
  else
    result = string.format("$ZED_WORKTREE_ROOT/%s", rel)
  end

  relative_cache[pathname] = result
  return result
end

local function collect_entries(wks, allowed)
  local entries = {}
  local seen = {}

  for prj in workspace.eachproject(wks) do
    if is_debuggable(prj) then
      local family_dir = get_family_dir(prj)
      local family_script = get_family_script_command(family_dir)
      local cwd_target = family_dir or prj.basedir or blu.rootdir
      local cwd = relative_to_root(cwd_target)

      for cfg in project.eachconfig(prj) do
        if should_include(cfg, allowed) then
          local key = string.format("%s:%s", prj.name, cfg.shortname)
          if not seen[key] then
            local args
            if cfg.debugargs and #cfg.debugargs > 0 then
              args = {}
              for _, value in ipairs(cfg.debugargs) do
                args[#args + 1] = value
              end
            end

            entries[#entries + 1] = {
              label = string.format("%s (%s)", prj.name, cfg.buildcfg),
              program = relative_to_root(cfg.buildtarget.abspath),
              cwd = cwd,
              args = args,
              build_command = build_command(prj.name, cfg.shortname, family_script),
            }

            seen[key] = true
          end
        end
      end
    end
  end

  table.sort(entries, function(a, b)
    return a.label < b.label
  end)

  return entries
end

local function write_array(values)
  p.push('[')
  for idx, value in ipairs(values) do
    local suffix = (idx == #values) and '' or ','
    p.w('"%s"%s', json_escape(value), suffix)
  end
  p.pop(']')
end

local function write_entries(entries)
  p.push('[')
  for idx, entry in ipairs(entries) do
    local suffix = (idx == #entries) and '' or ','
    p.push('{')
    p.w('"label": "%s",', json_escape(entry.label))
    p.w('"adapter": "CodeLLDB",')
    p.w('"request": "launch",')
    p.w('"program": "%s",', json_escape(entry.program))
    p.w('"cwd": "%s",', json_escape(entry.cwd))
    p.w('"args": ')
    write_array(entry.args or {})
    p.w(',')
    p.w('"build": {')
    p.w('  "command": "bash",')
    p.w('  "args": [')
    p.w('    "-lc",')
    p.w('    "%s"', json_escape(string.format("set -euo pipefail; %s", entry.build_command)))
    p.w('  ]')
    p.w('}')
    p.pop('}%s', suffix)
  end
  p.pop(']')
end

function m.onWorkspace(wks)
  local allowed = split_configs(_OPTIONS["zed-configs"])
  local entries = collect_entries(wks, allowed)

  local output = path.join(blu.rootdir, ".zed", "debug.json")
  p.generate(wks, output, function()
    write_entries(entries)
  end)
end

newaction({
  trigger = "zed-debug",
  description = "Generate Zed debugger configurations",
  onWorkspace = m.onWorkspace,
})

newoption({
  trigger = "zed-configs",
  value = "c1,c2",
  description = "Comma separated list of configurations to export (or 'all')",
  default = "debug,release",
})

newoption({
  trigger = "zed-fakeaction",
  value = "action",
  description = "Action name whose output layout should be mirrored",
  default = "gmake2",
})

return m
