// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/atomic.h>
#include <base/containers/hash_map.h>
#include <base/strings/xstring.h>

namespace {

TEST(HashMap, FindInsertAndOperatorBracket) {
  base::HashMap<u64, int> map;
  EXPECT_TRUE(map.find(1) == map.end());
  auto added = map.emplace(1, 10);
  EXPECT_TRUE(added.second);
  EXPECT_EQ(added.first->first, 1u);
  EXPECT_EQ(added.first->second, 10);
  EXPECT_FALSE(map.emplace(1, 99).second);
  EXPECT_EQ(map[1], 10);
  map[2] += 5;
  EXPECT_EQ(map.at(2), 5);
  EXPECT_EQ(map.size(), 2u);
  EXPECT_EQ(map.count(2), 1u);
  EXPECT_EQ(map.count(3), 0u);
  map.insert_or_assign(2, 7);
  EXPECT_EQ(map[2], 7);
}

TEST(HashMap, ReferencesSurviveRehash) {
  base::HashMap<u64, u64> map;
  u64* first = &map[0];
  *first = 1234;
  for (u64 i = 1; i < 10000; i++)
    map[i] = i;
  EXPECT_GT(map.bucket_count(), 16u);
  EXPECT_EQ(first, &map[0]);
  EXPECT_EQ(*first, 1234u);
  for (u64 i = 1; i < 10000; i++)
    ASSERT_EQ(map.find(i)->second, i);
}

TEST(HashMap, EraseWhileIteratingVisitsEveryElementOnce) {
  base::HashMap<u64, u64> map;
  for (u64 i = 0; i < 1000; i++)
    map[i] = i;
  u64 seen = 0;
  for (auto it = map.begin(); it != map.end();) {
    seen++;
    if (it->first % 2)
      it = map.erase(it);
    else
      ++it;
  }
  EXPECT_EQ(seen, 1000u);
  EXPECT_EQ(map.size(), 500u);
  for (u64 i = 0; i < 1000; i++)
    EXPECT_EQ(map.contains(i), i % 2 == 0);
  EXPECT_EQ(map.erase(0), 1u);
  EXPECT_EQ(map.erase(0), 0u);
}

TEST(HashMap, IterationCoversAllEntries) {
  base::HashMap<u64, u64> map;
  u64 sum = 0;
  for (u64 i = 0; i < 500; i++) {
    map[i * 7919] = i;
    sum += i;
  }
  u64 total = 0;
  for (auto& [key, value] : map)
    total += value;
  EXPECT_EQ(total, sum);
  const auto& cmap = map;
  u64 n = 0;
  for (auto it = cmap.begin(); it != cmap.end(); ++it)
    n++;
  EXPECT_EQ(n, 500u);
}

struct IdentityHash {
  mem_size operator()(u64 v) const { return static_cast<mem_size>(v); }
};

TEST(HashMap, SpreadsAnIdentityHashOfAlignedKeys) {
  base::HashMap<u64, int, IdentityHash> map;
  for (u64 i = 0; i < 4096; i++)
    map[i << 16] = 1;  // every key shares its low 16 bits
  EXPECT_EQ(map.size(), 4096u);
  for (u64 i = 0; i < 4096; i++)
    ASSERT_TRUE(map.contains(i << 16));
}

TEST(HashMap, HoldsValuesThatCannotMove) {
  base::HashMap<u64, base::Atomic<u32>> map;
  map[1].store(5);
  map.emplace(2, 7u);
  EXPECT_EQ(map[1].load(), 5u);
  EXPECT_EQ(map.find(2)->second.load(), 7u);
}

TEST(HashMap, StringKeysCopyAndMove) {
  base::HashMap<base::String, int> map;
  map["alpha"] = 1;
  map.emplace(base::String("beta"), 2);
  base::HashMap<base::String, int> copy = map;
  copy["alpha"] = 10;
  EXPECT_EQ(map["alpha"], 1);
  EXPECT_EQ(copy["alpha"], 10);
  base::HashMap<base::String, int> moved = base::move(copy);
  EXPECT_TRUE(copy.empty());
  EXPECT_EQ(moved.size(), 2u);
  moved.clear();
  EXPECT_TRUE(moved.empty());
  moved["gamma"] = 3;
  EXPECT_EQ(moved.size(), 1u);
}

TEST(HashSet, InsertFindErase) {
  base::HashSet<u64> set{1, 2, 3};
  EXPECT_EQ(set.size(), 3u);
  EXPECT_FALSE(set.insert(2).second);
  auto added = set.insert(4);
  EXPECT_TRUE(added.second);
  EXPECT_EQ(*added.first, 4u);
  EXPECT_TRUE(set.find(5) == set.end());
  EXPECT_EQ(set.erase(1), 1u);
  EXPECT_FALSE(set.contains(1));
  u64 sum = 0;
  for (u64 v : set)
    sum += v;
  EXPECT_EQ(sum, 2u + 3u + 4u);
  for (auto it = set.begin(); it != set.end();)
    it = set.erase(it);
  EXPECT_TRUE(set.empty());
}

}  // namespace
