// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/optional.h>

namespace {
TEST(Optional, AssingnValid) {
  base::Optional<bool> a(true);
  EXPECT_TRUE(a.has_value());
  EXPECT_TRUE(a.value());
}

TEST(Optional, AssingnEmpty) {
  base::Optional<bool> a;
  EXPECT_FALSE(a.has_value());
}

TEST(Optional, Copy) {
  base::Optional<bool> a;
  EXPECT_FALSE(a.has_value());

  base::Optional<bool> b = a;
  EXPECT_FALSE(b.has_value());
}
}  // namespace
