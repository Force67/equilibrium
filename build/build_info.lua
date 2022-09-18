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
  -- we want to convert the number to double, so first we strip the 'v' character
  -- then we remove the second (dots)
  local vnum = blu.version:sub(2)
  local index = vnum:find("%.")
  return vnum:sub(1, index + 1) .. (vnum:sub(index + 2, vnum:len()):gsub('%.', ''))
end

blu.git_branch = io_exec('git symbolic-ref --short -q HEAD')
blu.git_commit = io_exec('git rev-parse --short HEAD')
blu.git_commit_long = io_exec('git rev-parse HEAD')
-- See https://stackoverflow.com/questions/1404796/how-can-i-get-the-latest-tag-name-in-current-branch-in-git
blu.version = io_exec('git describe --tags ' .. io_exec('git rev-list --tags --max-count=1'))

-- include_meta adds build metadata to a given project, in the form of macros
-- and version info.
function include_meta(ico_path)
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

  filter("kind:WindowedApp or kind:ConsoleApp or kind:SharedLib")
    files((blu.builddir .. "/build_info.rc"))
    defines('EQ_ICON="%{prj.name}.ico"')
  filter{}
end