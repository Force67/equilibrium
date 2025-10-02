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

local function mock_action_path(value)
  local fake = _OPTIONS["zed-fakeaction"] or "gmake2"
  local escaped_action = escape_pattern(_ACTION)
  local escaped_fake = escape_pattern(fake)
  local pattern = string.format("/%s/", escaped_action)
  local replacement = string.format("/%s/", fake)
  local result, count = value:gsub(pattern, replacement)
  if count == 0 then
    result = result:gsub(escaped_action, fake)
  end
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

local function build_command(prj, cfg, family_dir)
  local parts = {}
  if family_dir and has_family_script(family_dir) then
    local rel_dir = path.getrelative(blu.rootdir, family_dir)
    table.insert(parts, string.format("./%s/make.sh %s", rel_dir, cfg.shortname))
  end
  table.insert(parts, string.format("make -C out/gmake2 %s config=%s", prj.name, cfg.shortname))

  local command = table.concat(parts, " && ")
  command = string.format("cd \"$ZED_WORKTREE_ROOT\" && %s", command)
  return command
end

local function relative_to_root(pathname)
  local rel = path.translate(path.getrelative(blu.rootdir, pathname))
  rel = mock_action_path(rel)
  if rel == "." then
    return "$ZED_WORKTREE_ROOT"
  end
  return string.format("$ZED_WORKTREE_ROOT/%s", rel)
end

local function collect_entries(wks, allowed)
  local entries = {}

  for prj in workspace.eachproject(wks) do
    if is_debuggable(prj) then
      local family_dir = get_family_dir(prj)
      local cwd_target = family_dir or prj.basedir or blu.rootdir
      local cwd = relative_to_root(cwd_target)

      for cfg in project.eachconfig(prj) do
        if should_include(cfg, allowed) then
          local key = string.format("%s:%s", prj.name, cfg.shortname)
          if not entries[key] then
            local program = relative_to_root(cfg.buildtarget.abspath)
            local build_cmd = build_command(prj, cfg, family_dir)

            local args
            if cfg.debugargs and #cfg.debugargs > 0 then
              args = {}
              for _, value in ipairs(cfg.debugargs) do
                args[#args + 1] = value
              end
            end

            entries[key] = {
              label = string.format("%s (%s)", prj.name, cfg.buildcfg),
              program = program,
              cwd = cwd,
              args = args,
              build_command = build_cmd,
            }
          end
        end
      end
    end
  end

  local ordered = {}
  for _, entry in pairs(entries) do
    ordered[#ordered + 1] = entry
  end

  table.sort(ordered, function(a, b)
    return a.label < b.label
  end)

  return ordered
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
