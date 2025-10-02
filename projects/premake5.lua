

local function profile_scope(label, fn)
  if blu and blu.profile and blu.profile.scope then
    return blu.profile.scope(label, fn)
  end
  return fn()
end

local function parse_filter()
  local env = os.getenv("PREMAKE_PROJECT_FILTER")
  if not env or env == "" then
    return nil
  end

  local filter = {}
  for entry in env:gmatch("[^,%s]+") do
    filter[entry] = true
  end
  return filter
end

local project_filter = parse_filter()

local function should_include_project(name)
  if not project_filter then
    return true
  end
  return project_filter[name] or project_filter["*"]
end

local function build_symlinks_nix(v)
  return profile_scope("symlink_nix:" .. tostring(v), function()
    if os.host() ~= "linux" then
      return
    end

    local symlink_source = "../../build/premake_linux.sh"
    local symlink_target = "./" .. v .. "/make.sh"
    -- if the symlink already exists, dont do anything
    if os.isfile(symlink_target) then
      return
    end

    os.execute("ln -s " .. symlink_source .. " " .. symlink_target)
  end)
end

local function build_symlinks_win(v)
  return profile_scope("symlink_win:" .. tostring(v), function()
    if os.host() ~= "windows" then
      return
    end

    local symlink_source = "../../build/premake_win.bat"
    local symlink_target = "./" .. v .. "\\make.bat"

    -- if the symlink already exists, don't do anything
    if os.isfile(symlink_target) then
      return
    end

    -- Command to create a symlink on Windows
    local command = 'cmd /c mklink "' .. symlink_target .. '" "' .. symlink_source .. '"'
    os.execute(command)
  end)
end

local function emit_symlinks(v)
  profile_scope("emit_symlinks:" .. tostring(v), function()
    if os.host() == "linux" then
      build_symlinks_nix(v)
    end

    if os.host() == "windows" then
      build_symlinks_win(v)
    end
  end)
end

local matches
profile_scope("projects:matchdirs", function()
  matches = os.matchdirs("*")
end)

for i,v in ipairs(matches) do
  profile_scope("projects:" .. tostring(v), function()
    if should_include_project(v) and os.isfile("./" .. v .. "/premake5.lua") then
      family(v)
      include("./" .. v)

      -- we also want to place a symlink for building and making the project
      -- this is a bit of a hack, but it works
      emit_symlinks(v)

      family("")
    end
  end)
end
