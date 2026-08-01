// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/algorithm.h>
#include <base/containers/vector.h>

namespace {

TEST(AlgorithmTest, EraseIfDropsMatchingElements) {
  base::Vector<i32> vec = {1, 2, 3, 4, 5, 6};

  const mem_size removed = base::EraseIf(vec, [](i32 v) { return v % 2 == 0; });

  EXPECT_EQ(removed, 3u);
  ASSERT_EQ(vec.size(), 3u);
  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[1], 3);
  EXPECT_EQ(vec[2], 5);
}

TEST(AlgorithmTest, EraseIfKeepsEverythingWhenNothingMatches) {
  base::Vector<i32> vec = {1, 3};

  EXPECT_EQ(base::EraseIf(vec, [](i32 v) { return v == 2; }), 0u);
  EXPECT_EQ(vec.size(), 2u);
}

TEST(AlgorithmTest, EraseIfCanEmptyTheContainer) {
  base::Vector<i32> vec = {2, 4};

  EXPECT_EQ(base::EraseIf(vec, [](i32) { return true; }), 2u);
  EXPECT_TRUE(vec.empty());
}

TEST(AlgorithmTest, EraseIfOnAnEmptyContainer) {
  base::Vector<i32> vec;

  EXPECT_EQ(base::EraseIf(vec, [](i32) { return true; }), 0u);
  EXPECT_TRUE(vec.empty());
}

TEST(AlgorithmTest, ClampBounds) {
  EXPECT_EQ(base::Clamp(5, 0, 10), 5);
  EXPECT_EQ(base::Clamp(-1, 0, 10), 0);
  EXPECT_EQ(base::Clamp(11, 0, 10), 10);
}

TEST(AlgorithmTest, SortOrdersAscending) {
  base::Vector<i32> vec = {3, 1, 2};

  base::Sort(vec.begin(), vec.end());

  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[2], 3);
}

}  // namespace
