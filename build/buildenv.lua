-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

blu.git_branch = ""
blu.git_commit = ""
blu.git_commit_long = ""

local f = io.popen('git symbolic-ref --short -q HEAD', 'r')
local temp = f:read("*a")
f:close()
-- sanitize
blu.git_branch = string.gsub(temp, '\n$', '')
f = io.popen('git rev-parse --short HEAD', 'r')
temp = f:read("*a")
f:close()
blu.git_commit = string.gsub(temp, '\n$', '')
f = io.popen('git rev-parse HEAD', 'r')
temp = f:read("*a")
f:close()
blu.git_commit_long = string.gsub(temp, '\n$', '')

function include_meta()
defines({
    ('GIT_BRANCH="' .. blu.git_branch .. '"'),
    ('GIT_COMMIT="' .. blu.git_commit .. '"'),
    ('GIT_COMMIT_LONG="' .. blu.git_commit_long .. '"'),
    ('BLU_NAME="%{wks.name}"'),
    ('BLU_NAME_WIDE=L"%{wks.name}"'),
    ('BLU_COMPANY="Stronkat"')
})
end