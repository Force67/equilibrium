-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

project("sample_app_cli")
  kind("ConsoleApp")
  flags("Maps") -- Enable Map File generation for codewrangler
  files({
    "premake5.lua",
    "**.h",
    "**.cc"
  })
  includedirs({
    ".",
    "../../sdk",
  })
  links("NemisisSDK")
	