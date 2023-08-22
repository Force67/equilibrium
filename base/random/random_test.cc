// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "random.h"
#include <gtest/gtest.h>

#define EXPECT_IN_RANGE(v, min, max) \
  EXPECT_GE(v, min);                 \
  EXPECT_LE(v, max)

#define ASSERT_IN_RANGE(v, min, max) \
  ASSERT_GE(v, min);                 \
  ASSERT_LE(v, max)

namespace {
TEST(Random, RandomIntMinMax) {
  i32 random_int = base::RandomInt();
  EXPECT_TRUE(base::TestMinMax(random_int));
}

TEST(Random, RandomIntRange) {
  for (i32 i = -100; i < 100; i++) {
    EXPECT_IN_RANGE(base::RandomInt(-100, i), -100, 100);
  }
}

TEST(Random, RandomUnsignedIntMinMax) {
  u32 random_int = base::RandomUint();
  EXPECT_TRUE(base::TestMinMax(random_int));
}

TEST(Random, RandomUnsignedIntRange) {
  for (u32 i = 0; i < 100; i++) {
    EXPECT_IN_RANGE(base::RandomUint(0, i), 0, 100);
  }
}
}  // namespace