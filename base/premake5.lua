-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

-- this will replace the crt init code with our own bootcode,
-- so we can perform a number of optimizations and security enhancements
function enable_base_crt_bootstrap()
  defines("BASE_ENABLE_CRT_BOOTSTRAP")

  -- this should be one always without semicolon, else the path breaks!
  includedirs("$(VCToolsInstallDir)/crt/src/vcruntime")
end

local function base_project()
  filter("configurations:Profile")
    dependencies("tracysdk")
  filter{}

  dependencies("googlemock")  
  links({"fmtlib"})
  defines({
    "BASE_IMPLEMENTATION",
    "TRACY_HAS_CALLSTACK"})
  includedirs({".", blu.rootdir, blu.extdir .. "/fmt/include"})
end

local function base_library()
  enable_base_crt_bootstrap()
  base_project()
  files({"**.cc", "**.h", "**.in", "**.inl"})
  removefiles({
    "**_test.cc",
    "allocator/memory_unittests_main.cc"})
end

project("base")
  kind("StaticLib")
  base_library()

project("base_shared")
  kind("SharedLib")
  base_library()
  defines("BUILDING_DLL")

project("base_unittests")
  kind("ConsoleApp")
  base_project()

  filter("system:windows")
    dependencies({
      "gtest-memleak-detector",
      "stackwalker"
    })
    defines("BASE_TESTING_GTEST_MEMLEAK_DETECTION")
  filter{}
  
  add_generic_test_main()
  files({"**.cc", "**.h", "**.in", "**.inl"})
  removefiles({
    "allocator/**_test.cc", 
    "allocator/memory_unittests_main.cc"})

-- base is a special case where we cannot rely on the unittest model
-- found in components, so we have to manually add the test project
project("base_memory_unittests")
  kind("ConsoleApp")
  files({
    "allocator/**.cc",
    "allocator/**.h",
    "allocator/**.in",
    "allocator/**.inl",
    "containers/linked_list.cc",
    "containers/linked_list.h",
    "threading/spinning_mutex.h",
    "threading/spinning_mutex.cc"
  })
  base_project()