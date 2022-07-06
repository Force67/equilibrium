-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

project("NemisisSDK")
  kind("StaticLib")
  defindes("BUILDING_NEMISIS_SDK")
  files({
    "**.cc",
    "**.h",
  })
  includedirs({
    ".",
    blu.rootdir
  })