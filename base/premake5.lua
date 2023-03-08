-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

-- this will replace the crt init code with our own bootcode,
-- so we can perform a number of optimizations and security enhancements
function enable_base_crt_bootstrap()
  filter("system:windows")
    defines("BASE_ENABLE_CRT_BOOTSTRAP")
    -- this should be one always without semicolon, else the path breaks!
    includedirs("$(VCToolsInstallDir)/crt/src/vcruntime")

    linkoptions({
      -- use our own main instead of winmain
      "/ENTRY:EquilibriumExecutableMain"
    })
  filter{}
end

local function base_project()
  filter("configurations:Profile")
    dependencies("tracysdk")
  filter{}

  dependencies({
    "googlemock",
    "fmtlib"
  })  
  defines({
    "BASE_IMPLEMENTATION",
    "TRACY_HAS_CALLSTACK"})
  includedirs({".", blu.rootdir})
end

local function base_library()
  base_project()
  files({"**.cc", "**.h", "**.in", "**.inl"})
  removefiles({
    "**_test.cc",
    "allocator/memory_unittests_main.cc"})
end

project("base")
  kind("StaticLib")
  base_library()
  sdktargetdir("base_sdk")
  sdkfiles({"**.h", "**.in"})
  enable_base_crt_bootstrap() -- we need a port for shared libraries!

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
    defines({
      "_CRTDBG_MAP_ALLOC",
      "BASE_TESTING_GTEST_MEMLEAK_DETECTION"
    })
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