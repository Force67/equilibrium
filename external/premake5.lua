-- Copyright (C) 2021 Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

-- ida flavored qt for the integrastion plugin
require("premake-idaqt/qt")
qt = premake.modules.qt

group("Dependencies")

function include_skia()
  defines("SK_GL")
  links("skia")
  includedirs({
    blu.extdir .. "/skia_sdk",
    blu.extdir .. "/skia_sdk/include",
  })
  -- Our debug mode differs from the one used by SKIA
  -- so we need to use the release static libs
  libdirs(blu.extdir .. "/skia_sdk/lib/release_static")
end

project("capstone")
  kind("StaticLib")
  language("C++")
  includedirs("./capstone")
  pubincludedirs("capstone/include")
  files({
    "capstone/*.c",
    "capstone/*.h",
  })

project("tracysdk")
  kind("StaticLib")
  language("C++")
  pubdefines("TRACY_ENABLE")
  pubincludedirs("./tracy")
  files("tracy/TracyClient.cpp")

  -- v 8.2.1.+
  --pubincludedirs("./tracy/public")
  --files({
    --"tracy/public/TracyClient.cpp",
  --})

project("zydis")
  kind("StaticLib")
  language("C++")
  pubincludedirs({
    "zydis/include",
    "zydis/dependencies/zycore/include",
  })
  includedirs("zydis/src")
  pubdefines({
    "ZYDIS_STATIC_BUILD",
    "ZYCORE_STATIC_BUILD"})
  files({
    "zydis/src/*.c",
    "zydis/include/**.h",
    "zydis/dependencies/zycore/include/**.h",
    "zydis/dependencies/zycore/src/**.c",
  })

project("mbedtls")
  kind("StaticLib")
  language("C")
  includedirs({
    "mbedtls/include",
    "mbedtls/library"
  })
  pubincludedirs({
    "mbedtls/include",
  })
  pubdefines({
    "MBEDTLS_ALLOW_PRIVATE_ACCESS"
  })
  files({
    "mbedtls/library/*.c",
    "mbedtls/library/*.h",
  })

project("sqlite")
  kind("StaticLib")
  language("C")
  pubincludedirs("./sqlite")
  files({
    "sqlite/*.h",
    "sqlite/*.c"
  })
  pubdefines("SQLITE_ENABLE_SQLLOG")
  filter("configurations:Debug")
  pubdefines("SQLITE_DEBUG")

project("fmtlib")
  language("C++")
  kind("StaticLib")
  pubincludedirs({
    "./fmt/include",
  })
  files({
    "fmt/src/format.cc",
    "fmt/src/os.cc",
  })

project("googlemock")
  language("C++")
  kind("StaticLib")
  pubincludedirs({
    "googletest/googlemock/include",
    "googletest/googletest/include",
    "googletest/googlemock",
    "googletest/googletest"
  })
  files({
    "googletest/googlemock/src/gmock-all.cc",
    "googletest/googletest/src/gtest-all.cc",
    --"googletest/googlemock/src/*.cc",
    --"googletest/googletest/src/*.cc",
    "googletest/googlemock/include/gmock/**.h",
    --"googletest/googletest/include/gtes/**.h",
  })

project("stackwalker")
  language("C++")
  kind("StaticLib")
  pubincludedirs({
    "stackwalker/Main/StackWalker",
  })
  files({
    "stackwalker/Main/StackWalker/StackWalker.cpp",
    "stackwalker/Main/StackWalker/StackWalker.h",
  })

project("gtest-memleak-detector")
  language("C++")
  kind("StaticLib")
  pubincludedirs({
    "googletest/googletest/include",
    "googletest/googletest",
    "gtest-memleak-detector/include",
    "stackwalker/Main",
  })
  files({
    "gtest-memleak-detector/include/gtest_memleak_detector/gtest_memleak_detector.h",
    "gtest-memleak-detector/src/*.cpp",
  })

project("imgui")
  language("C++")
  kind("StaticLib")
  pubincludedirs({
    "./imgui",
    "glew/include",
    "glfw/include",
  })
  pubdefines({
    "GLEW_STATIC"
  })
  files({
    "imgui/imgui.cpp",
    "imgui/imgui.h",
    "imgui/imgui_draw.cpp",
    "imgui/imgui_internal.h",
    "imgui/imgui_tables.cpp",
    "imgui/imgui_widgets.cpp",
    "imgui/imstb_rectpack.h",
    "imgui/imstb_textedit.h",
    "imgui/imstb_truetype.h"
  })

project("lz4")
  language("C")
  kind("StaticLib")
  pubincludedirs({
    "./lz4/lib",
  })
  files({
    "lz4/lib/*.c",
    "lz4/lib/*.h"
  })
