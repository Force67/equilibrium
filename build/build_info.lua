-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

-- global build_info
-- should maybe get refactored as a global struct

blu.git_branch = ""
blu.git_commit = ""
blu.git_commit_long = ""
blu.version = ""

local function io_exec(command)
  local pipe = io.popen(command, 'r')
  local rval = pipe:read("*a")
  pipe:close()
  return string.gsub(rval, '\n$', '')
end

local function to_double(version)
    -- Check if version is nil or not a string
    if not version or type(version) ~= "string" then
        return 0
    end

    -- Check if version starts with 'v'
    if not version:match("^v%d") then
        return 0
    end

    -- Strip the 'v' character
    local vnum = version:sub(2)

    -- Find first dot
    local index = vnum:find("%.")
    if not index then
        return 0
    end

    -- Ensure there are digits after the first dot
    if not vnum:match("^%d+%.%d") then
        return 0
    end

    -- Convert to double format
    local result = vnum:sub(1, index + 1) .. (vnum:sub(index + 2, vnum:len()):gsub('%.', ''))

    -- Verify the result is a valid number
    local number = tonumber(result)
    if not number then
        return 0
    end

    return number
end

local is_ci = os.getenv('BLU_IS_CI_BUILD') == 'true'
if is_ci then
  blu.git_branch = os.getenv('GITHUB_REF') or "<none>"
  blu.git_commit = os.getenv('GITHUB_SHA') or "<none>"
  blu.git_commit_long = os.getenv('GITHUB_SHA') or "<none>"
  blu.version = os.getenv('GITHUB_TAG') or "0.0.1"
else
  blu.git_branch = io_exec('git symbolic-ref --short -q HEAD')
  blu.git_commit = io_exec('git rev-parse --short HEAD')
  blu.git_commit_long = io_exec('git rev-parse HEAD')
  -- See https://stackoverflow.com/questions/1404796/how-can-i-get-the-latest-tag-name-in-current-branch-in-git
  blu.version = io_exec('git describe --tags ' .. io_exec('git rev-list --tags --max-count=1'))
end

-- include_meta adds build metadata to a given project, in the form of macros
-- and version info.
function include_meta(ico_path, asset_files)
  defines({
    ('GIT_BRANCH="' .. blu.git_branch .. '"'),
    ('GIT_COMMIT="' .. blu.git_commit .. '"'),
    ('GIT_COMMIT_LONG="' .. blu.git_commit_long .. '"'),
    ('EQ_NAME="%{wks.name}"'),
    ('EQ_NAME_WIDE=L"%{wks.name}"'),
    ('EQ_COMPANY="VH Tech"'),
    ('EQ_VERSION_STR="' .. blu.version .. '"'),
    -- delete the first character 'v'
    ('EQ_VERSION=' .. to_double(blu.version))
  })

  filter({"system:windows", "kind:WindowedApp or kind:ConsoleApp or kind:SharedLib"})
    files({
      (blu.builddir .. "/build_info.rc"),
      (blu.builddir .. "/win_app.manifest")
    })
    defines('EQ_ICON="%{prj.name}.ico"')

    if asset_files then
      for index, asset_file in ipairs(asset_files) do
        defines('EQ_ASSET_' .. index .. '="' .. asset_file .. '"')
      end
    end
  filter{}
end
