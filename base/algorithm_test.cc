// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/containers/array.h>
#include <base/containers/hash_map.h>
#include <base/containers/map.h>
#include <base/containers/vector.h>
#include <base/math/value_bounds.h>

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

namespace {
struct Keyed {
  i32 key;
  i32 payload;
};

TEST(AlgorithmBounds, LowerAndUpperBoundWithComparator) {
  Keyed items[] = {{1, 0}, {3, 1}, {3, 2}, {3, 3}, {7, 4}};
  auto before = [](const Keyed& k, i32 v) { return k.key < v; };
  auto after = [](i32 v, const Keyed& k) { return v < k.key; };
  EXPECT_EQ(base::LowerBound(items, items + 5, 3, before), items + 1);
  EXPECT_EQ(base::UpperBound(items, items + 5, 3, after), items + 4);
  EXPECT_EQ(base::LowerBound(items, items + 5, 0, before), items);
  EXPECT_EQ(base::LowerBound(items, items + 5, 9, before), items + 5);
  EXPECT_EQ(base::UpperBound(items, items + 5, 7, after), items + 5);
  EXPECT_EQ(base::LowerBound(items, items, 3, before), items);
}

TEST(AlgorithmBounds, UpperBoundPlain) {
  i32 v[] = {1, 2, 2, 2, 5};
  EXPECT_EQ(base::UpperBound(v, v + 5, 2), v + 4);
  EXPECT_EQ(base::UpperBound(v, v + 5, 0), v);
  EXPECT_EQ(base::UpperBound(v, v + 5, 5), v + 5);
}
TEST(AlgorithmAdditions, CopyNAndNoneOf) {
  int src[4] = {1, 2, 3, 4};
  int dst[4] = {};
  EXPECT_EQ(base::CopyN(src, 3, dst), dst + 3);
  EXPECT_EQ(dst[2], 3);
  EXPECT_EQ(dst[3], 0);
  EXPECT_TRUE(base::NoneOf(src, src + 4, [](int v) { return v > 4; }));
  EXPECT_FALSE(base::NoneOf(src, src + 4, [](int v) { return v == 2; }));
}

TEST(AlgorithmAdditions, EraseIfOnANodeContainer) {
  base::HashMap<int, int> map;
  for (int i = 0; i < 10; i++)
    map[i] = i;
  EXPECT_EQ(base::EraseIf(map, [](const auto& kv) { return kv.second % 3 == 0; }),
            4u);
  EXPECT_EQ(map.size(), 6u);
  EXPECT_FALSE(map.contains(3));
  base::Vector<int> vec{1, 2, 3, 4};
  EXPECT_EQ(base::EraseIf(vec, [](int v) { return v % 2 == 0; }), 2u);
  EXPECT_EQ(vec.size(), 2u);
}

TEST(AlgorithmAdditions, MinMaxOfAList) {
  EXPECT_EQ(base::Min({5, 2, 9}), 2);
  EXPECT_EQ(base::Max({5, 2, 9}), 9);
  EXPECT_EQ(base::Max<u64>({1, 7}), 7u);
}

TEST(AlgorithmAdditions, ArrayAndVectorOrderLexicographically) {
  base::Array<int, 3> a{1, 2, 3}, b{1, 3, 0};
  EXPECT_TRUE(a < b);
  EXPECT_FALSE(b < a);
  EXPECT_FALSE(a < a);
  base::Vector<int> x{1, 2}, y{1, 2, 0};
  EXPECT_TRUE(x < y);
  EXPECT_FALSE(y < x);
  EXPECT_TRUE((base::Vector<int>{0, 9} < base::Vector<int>{1}));
}

TEST(AlgorithmAdditions, PrevAndNextLeaveTheIteratorAlone) {
  base::Map<int, int> map;
  map[1] = 10;
  map[2] = 20;
  auto it = map.find(2);
  EXPECT_EQ(base::Prev(it)->first, 1);
  EXPECT_EQ(it->first, 2);
  EXPECT_TRUE(base::Next(it) == map.end());
  int v[3] = {1, 2, 3};
  EXPECT_EQ(*base::Next(v), 2);
}

}  // namespace
