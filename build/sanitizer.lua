-- Taken from https://github.com/premake/premake-core/issues/1595
-- here until the issue gets closed.
-- Register api command
local Field = require('field')

Field.register({
    name = "enableASAN",
    scope = "config",
    kind = "string",
    allowed = { "true", "false" }
})