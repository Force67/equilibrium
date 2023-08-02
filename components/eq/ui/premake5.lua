-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

local function include_ui()
  dependencies({
    "fmtlib",
    "imgui",
    "eq_gpu" -- gpu component
  })
  includedirs("../../")
  -- define our own imgui config file.
  pubdefines({
    "IMGUI_USER_CONFIG=<eq/ui/imgui/imgui_config.h>",
    "SK_USER_CONFIG_HEADER=<eq/ui/skia/skia_config.h>"
  })
end

component2("eq_ui")
  files({
    "**.cc",
    "**.h",
    "**.inl"
  })
  include_ui()

unittest2("eq_ui:platformtests")
  files({
    "platform/**.cc",
    "platform/**.h",
    "display/**.cc",
    "display/**.h",
  })
  include_ui()
  add_generic_test_main()

unittest2("eq_ui:gammatests")
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

unittest2("eq_ui:layout")
  files({
    "test/run_all_tests.cc",
    "test/ui_test_suite.cc",
    "test/ui_test_suite.h",
    "layout/*.cc",
    "layout/*.h"
  })
  include_ui()