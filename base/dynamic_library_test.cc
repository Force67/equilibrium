// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/dynamic_library.h>

namespace {
TEST(DynamicLibrary, Load) {
  const base::DynamicLibrary lib;
  ASSERT_FALSE(lib.loaded());
}
}  // namespace