// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/optional.h>

namespace {
TEST(Optional, AssingnValid) {
  base::Optional<bool> a(true);
  EXPECT_FALSE(a.failed());
  EXPECT_TRUE(a.value());
}

TEST(Optional, AssingnEmpty) {
  base::Optional<bool> a;
  EXPECT_TRUE(a.failed());
}

TEST(Optional, Copy) {
  base::Optional<bool> a;
  EXPECT_TRUE(a.failed());

  base::Optional<bool> b = a;
  EXPECT_TRUE(b.failed());
}
}  // namespace