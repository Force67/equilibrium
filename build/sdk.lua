-- Copyright (C) Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.
-- Implement dependency extension model for premake.
local p = premake
local api = p.api

api.register({
  name = "sdkfiles",
  scope = "config",
  kind = "list:file",
  tokens = true,
})

-- TODO: needs some kind of SDK action to export the SDK files.