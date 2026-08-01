// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/containers/unordered_set.h>
#include <base/strings/xstring.h>

namespace {

TEST(UnorderedSetTest, InsertReportsNovelty) {
  base::UnorderedSet<i32> set;

  EXPECT_TRUE(set.insert(7));
  EXPECT_FALSE(set.insert(7));
  EXPECT_EQ(set.size(), 1);
}

TEST(UnorderedSetTest, ContainsAndCount) {
  base::UnorderedSet<i32> set;
  set.insert(1);
  set.insert(2);

  EXPECT_TRUE(set.contains(1));
  EXPECT_FALSE(set.contains(3));
  EXPECT_EQ(set.count(2), 1);
  EXPECT_EQ(set.count(3), 0);
}

TEST(UnorderedSetTest, EraseAndClear) {
  base::UnorderedSet<i32> set;
  set.insert(4);
  set.insert(5);

  EXPECT_TRUE(set.erase(4));
  EXPECT_FALSE(set.erase(4));
  EXPECT_EQ(set.size(), 1);

  set.clear();
  EXPECT_TRUE(set.empty());
}

TEST(UnorderedSetTest, IteratesEveryKey) {
  base::UnorderedSet<i32> set;
  for (i32 i = 0; i < 16; ++i) set.insert(i);

  i32 seen = 0;
  for (i32 key : set) {
    EXPECT_GE(key, 0);
    EXPECT_LT(key, 16);
    ++seen;
  }
  EXPECT_EQ(seen, 16);
}

TEST(UnorderedSetTest, IteratesWhenConst) {
  base::UnorderedSet<i32> set;
  set.insert(3);
  const auto& const_set = set;

  mem_size seen = 0;
  for (i32 key : const_set) {
    EXPECT_EQ(key, 3);
    ++seen;
  }
  EXPECT_EQ(seen, 1);
}

TEST(UnorderedSetTest, HoldsStringKeys) {
  base::UnorderedSet<base::String> set;
  set.insert("alpha");
  set.insert("beta");

  EXPECT_TRUE(set.contains("alpha"));
  EXPECT_FALSE(set.insert("alpha"));
  EXPECT_EQ(set.size(), 2);
}

TEST(UnorderedSetTest, BraceInitialization) {
  base::UnorderedSet<base::String> set = {"alpha", "beta", "alpha"};

  EXPECT_EQ(set.size(), 2u);
  EXPECT_TRUE(set.contains("beta"));
}

TEST(UnorderedSetTest, GrowsPastTheInitialBuckets) {
  base::UnorderedSet<i32> set;
  set.reserve(128);
  for (i32 i = 0; i < 512; ++i) set.insert(i);

  EXPECT_EQ(set.size(), 512);
  for (i32 i = 0; i < 512; ++i) EXPECT_TRUE(set.contains(i));
}

}  // namespace
