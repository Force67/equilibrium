// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/filesystem/path.h>
#include <base/dynamic_library.h>

namespace {
TEST(DynamicLibrary, EmptyLoad) {
  const base::DynamicLibrary lib;
  ASSERT_FALSE(lib.loaded());
}

TEST(DynamicLibrary, Load) {
  const base::DynamicLibrary lib("/lib/x86_64-linux-gnu/libstdc++.so");
  ASSERT_TRUE(lib.loaded());
}

TEST(DynamicLibrary, LoadExisting) {
  const base::Path kPath("libc.so");
  const base::DynamicLibrary lib(kPath);
  ASSERT_TRUE(lib.loaded());
}
}  // namespace