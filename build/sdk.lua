-- Copyright (C) 2023 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.
-- Implement dependency extension model for premake.
local p = premake
local api = p.api
-- sdktargetdirs must be set for anythging to happen at all.
-- it points to a sub folder of the out dir.
-- e.g.supply "sdk" to get C:\Users\user\Projects\equilibrium\out\vs2022\bin\Debug\equilibrium\<sdk> on windows
-- we don't want to resolve the one layer deep path as a directory here, so we keep it dumb by using a string kind...
api.register({
  name = "sdktargetdir",
  scope = "config",
  kind = "string",
  tokens = true,
})

api.register({
  name = "sdkfiles",
  scope = "config",
  kind = "list:file",
  tokens = true,
})