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

TEST(Optional, NullOptConstructsDisengaged) {
  base::Optional<f32> a = base::nullopt;

  EXPECT_FALSE(a.has_value());
  EXPECT_TRUE(a == base::nullopt);
}

TEST(Optional, NullOptResetsAnEngagedOptional) {
  base::Optional<f32> a = 1.5f;
  ASSERT_TRUE(a.has_value());

  a = base::nullopt;

  EXPECT_FALSE(a.has_value());
  EXPECT_FALSE(a != base::nullopt);
}

TEST(Optional, NullOptIsPickedByTheConditionalOperator) {
  const bool disengage = true;
  base::Optional<i32> a = disengage ? base::nullopt : base::Optional<i32>(3);

  EXPECT_FALSE(a.has_value());
}

TEST(Optional, Copy) {
  base::Optional<bool> a;
  EXPECT_FALSE(a.has_value());

  base::Optional<bool> b = a;
  EXPECT_FALSE(b.has_value());
}
}  // namespace
