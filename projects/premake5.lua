

local function build_symlinks_nix(v)
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
end

local function build_symlinks_win(v)
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
end

local function emit_symlinks(v)
  if os.host() == "linux" then
    build_symlinks_nix(v)
  end

  if os.host() == "windows" then
    build_symlinks_win(v)
  end
end

matches = os.matchdirs("*")
for i,v in ipairs(matches) do
  if os.isfile("./" .. v .. "/premake5.lua") then
    family(v)
    include("./" .. v)

    -- we also want to place a symlink for building and making the project
    -- this is a bit of a hack, but it works
    emit_symlinks(v)
    
    family("")
  end
end