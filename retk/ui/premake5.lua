-- Copyright (C) 2021 Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

local function include_ui()
    dependencies({
        "fmtlib",
        "glew",
        "glfw",
        "imgui",
    })
    include_skia()
    -- define our own imgui config file.
    pubdefines("IMGUI_USER_CONFIG=<ui/imgui/imgui_config.h>")
end

component("ui")
    files({
        "**.cc",
        "**.h",
        "**.inl"
    })
    include_ui()

unittest("ui:gammatests")
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

unittest("ui:layout")
    files({
        "test/run_all_tests.cc",
        "test/ui_test_suite.cc",
        "test/ui_test_suite.h",
        "layout/*.cc",
        "layout/*.h"
    })
    include_ui()