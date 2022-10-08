-- Copyright (C) 2021 Force67 <github.com/Force67>.
-- For licensing information see LICENSE at the root of this distribution.

local function build_gpu()
  dependencies("fmtlib")
  includedirs("..")
  files({
    "readme.md",
    "**.cc",
    "**.c",
    "**.h",
    "**.inl"})
  filter("system:windows")
    defines("VK_USE_PLATFORM_WIN32_KHR")
  filter{}
end

component2("gpu")
  build_gpu()

unittest2("gpu:vulkan_test")
  build_gpu()