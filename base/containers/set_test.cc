// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/containers/set.h>

namespace {

TEST(SetTest, InsertAndContains) {
  base::Set<i32> set;
  set.Insert(3);
  set.Insert(1);
  set.Insert(3);

  EXPECT_EQ(set.size(), 2u);
  EXPECT_TRUE(set.Contains(1));
  EXPECT_FALSE(set.Contains(2));
}

TEST(SetTest, Remove) {
  base::Set<i32> set;
  set.Insert(5);

  EXPECT_TRUE(set.Remove(5));
  EXPECT_FALSE(set.Remove(5));
  EXPECT_TRUE(set.empty());
}

TEST(SetTest, LowercaseSpellingsMatchThePascalCaseOnes) {
  base::Set<i32> set;
  set.insert(7);

  EXPECT_TRUE(set.contains(7));
  EXPECT_EQ(set.count(7), 1u);
  EXPECT_EQ(set.count(8), 0u);
  EXPECT_TRUE(set.erase(7));
  EXPECT_EQ(set.count(7), 0u);
}

TEST(SetTest, ClearEmptiesTheSet) {
  base::Set<i32> set;
  for (i32 i = 0; i < 8; ++i) set.insert(i);
  ASSERT_EQ(set.size(), 8u);

  set.clear();

  EXPECT_TRUE(set.empty());
  EXPECT_EQ(set.size(), 0u);
  EXPECT_FALSE(set.contains(3));
}

TEST(SetTest, BraceInitialization) {
  base::Set<i32> set = {3, 1, 3, 2};

  EXPECT_EQ(set.size(), 3u);
  EXPECT_TRUE(set.contains(1));
  EXPECT_TRUE(set.contains(3));
  EXPECT_FALSE(set.contains(4));
}

TEST(SetTest, IteratesInSortedOrder) {
  base::Set<i32> set;
  set.insert(5);
  set.insert(1);
  set.insert(3);

  i32 previous = -1;
  mem_size seen = 0;
  for (auto it = set.begin(); it != set.end(); ++it) {
    EXPECT_GT(*it, previous);
    previous = *it;
    ++seen;
  }
  EXPECT_EQ(seen, 3u);
}

}  // namespace
