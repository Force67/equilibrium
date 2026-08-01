// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/containers/adapters.h>
#include <base/containers/vector.h>

namespace {

// Regression: the adapter used to hand out begin()/end(), so every caller
// quietly iterated forwards. CreateDirectory depended on it and could not
// create a nested path because it tried the deepest component first.
TEST(AdaptersTest, ReversedWalksBackToFront) {
  base::Vector<i32> vec = {1, 2, 3};

  base::Vector<i32> seen;
  for (i32 value : base::Reversed(vec))
    seen.push_back(value);

  ASSERT_EQ(seen.size(), 3u);
  EXPECT_EQ(seen[0], 3);
  EXPECT_EQ(seen[1], 2);
  EXPECT_EQ(seen[2], 1);
}

TEST(AdaptersTest, ReversedOverASingleElement) {
  base::Vector<i32> vec = {7};

  mem_size seen = 0;
  for (i32 value : base::Reversed(vec)) {
    EXPECT_EQ(value, 7);
    ++seen;
  }
  EXPECT_EQ(seen, 1u);
}

TEST(AdaptersTest, ReversedOverAnEmptyContainerDoesNothing) {
  base::Vector<i32> vec;

  for (i32 value : base::Reversed(vec)) {
    (void)value;
    FAIL() << "empty container yielded an element";
  }
}

}  // namespace
