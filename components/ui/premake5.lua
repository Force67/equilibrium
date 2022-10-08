-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

local function include_ui()
  dependencies({
    "fmtlib",
    "imgui",
    "gpu" -- gpu component
  })
  include_skia()
  -- define our own imgui config file.
  pubdefines({
    "IMGUI_USER_CONFIG=<ui/imgui/imgui_config.h>",
    "SK_USER_CONFIG_HEADER=<ui/skia/skia_config.h>"
  })
end

component2("ui")
  files({
    "**.cc",
    "**.h",
    "**.inl"
  })
  include_ui()

unittest2("ui:platformtests")
  files({
    "platform/**.cc",
    "platform/**.h",
    "display/**.cc",
    "display/**.h",
  })
  include_ui()
  add_generic_test_main()

unittest2("ui:gammatests")
  files({
    "test/run_all_tests.cc",
    "test/ui_test_suite.cc",
    "test/ui_test_suite.h",
    -- for now, just to get tests working...
    "gamma/*.cc",
    "gamma/*.h",
    "gamme/*.inl"
  })
  include_ui()

unittest2("ui:layout")
  files({
    "test/run_all_tests.cc",
    "test/ui_test_suite.cc",
    "test/ui_test_suite.h",
    "layout/*.cc",
    "layout/*.h"
  })
  include_ui()