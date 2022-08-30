-- Copyright (C) 2022 Vincent Hengel.
-- For licensing information see LICENSE at the root of this distribution.

local function base_project()
  
  filter("configurations:Profile")
    dependencies("tracysdk")
  filter{}

  dependencies("googlemock")
  links({"fmtlib"})
  defines("BASE_IMPLEMENTATION")
  includedirs({".", blu.rootdir, blu.extdir .. "/fmt/include"})
end

group("Base")
  project("base")
  kind("StaticLib")
  base_project()
  files({"**.cc", "**.h", "**.in", "**.inl"})
  removefiles({
    "**_test.cc",
    "allocator/memory_unittests_main.cc"})

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