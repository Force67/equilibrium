// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/containers/unordered_map.h>
#include <base/numeric_limits.h>
#include <base/strings/xstring.h>

namespace {

TEST(UnorderedMapTest, InsertAndFind) {
  base::UnorderedMap<i32, i32> map;

  auto inserted = map.insert(1, 10);
  ASSERT_NE(inserted.first, nullptr);
  EXPECT_TRUE(inserted.second);
  EXPECT_EQ(map.size(), 1u);

  ASSERT_NE(map.find(1), nullptr);
  EXPECT_EQ(*map.find(1), 10);
  EXPECT_EQ(map.find(2), nullptr);
}

TEST(UnorderedMapTest, InsertDoesNotOverwriteAnExistingKey) {
  base::UnorderedMap<i32, i32> map;
  map.insert(1, 10);

  auto again = map.insert(1, 99);

  EXPECT_FALSE(again.second);
  EXPECT_EQ(*map.find(1), 10);
  EXPECT_EQ(map.size(), 1u);
}

TEST(UnorderedMapTest, SubscriptInsertsAndOverwrites) {
  base::UnorderedMap<i32, i32> map;

  map[3] = 30;
  EXPECT_EQ(map.size(), 1u);
  map[3] = 31;

  EXPECT_EQ(map.size(), 1u);
  EXPECT_EQ(*map.find(3), 31);
}

TEST(UnorderedMapTest, SubscriptValueInitializesANewKey) {
  base::UnorderedMap<i32, i32> map;

  EXPECT_EQ(map[7], 0);
  EXPECT_EQ(map.size(), 1u);
}

TEST(UnorderedMapTest, ContainsAndCount) {
  base::UnorderedMap<i32, i32> map;
  map.insert(1, 10);

  EXPECT_TRUE(map.contains(1));
  EXPECT_FALSE(map.contains(2));
  EXPECT_EQ(map.count(1), 1u);
  EXPECT_EQ(map.count(2), 0u);
}

TEST(UnorderedMapTest, AtReturnsThePresentValue) {
  base::UnorderedMap<i32, i32> map;
  map.insert(1, 10);

  EXPECT_EQ(map.at(1), 10);
  map.at(1) = 11;
  EXPECT_EQ(*map.find(1), 11);

  const auto& const_map = map;
  EXPECT_EQ(const_map.at(1), 11);
}

TEST(UnorderedMapTest, Erase) {
  base::UnorderedMap<i32, i32> map;
  map.insert(1, 10);
  map.insert(2, 20);

  EXPECT_TRUE(map.erase(1));
  EXPECT_FALSE(map.erase(1));
  EXPECT_EQ(map.size(), 1u);
  EXPECT_EQ(map.find(1), nullptr);
  EXPECT_EQ(*map.find(2), 20);
}

TEST(UnorderedMapTest, LookupSurvivesTombstones) {
  // Erase leaves tombstones; a key probed past one must still be reachable.
  base::UnorderedMap<i32, i32> map;
  for (i32 i = 0; i < 64; ++i) map.insert(i, i * 2);
  for (i32 i = 0; i < 64; i += 2) map.erase(i);

  for (i32 i = 1; i < 64; i += 2) {
    ASSERT_NE(map.find(i), nullptr) << "lost key " << i;
    EXPECT_EQ(*map.find(i), i * 2);
  }
  EXPECT_EQ(map.size(), 32u);
}

TEST(UnorderedMapTest, GrowsAndRehashes) {
  base::UnorderedMap<i32, i32> map;
  for (i32 i = 0; i < 1000; ++i) map.insert(i, i);

  EXPECT_EQ(map.size(), 1000u);
  for (i32 i = 0; i < 1000; ++i) {
    ASSERT_NE(map.find(i), nullptr) << "lost key " << i;
    EXPECT_EQ(*map.find(i), i);
  }
}

TEST(UnorderedMapTest, Clear) {
  base::UnorderedMap<i32, i32> map;
  map.insert(1, 10);

  map.clear();

  EXPECT_TRUE(map.empty());
  EXPECT_EQ(map.find(1), nullptr);
}

TEST(UnorderedMapTest, StringKeys) {
  base::UnorderedMap<base::String, i32> map;
  map.insert("alpha", 1);
  map["beta"] = 2;

  ASSERT_NE(map.find("alpha"), nullptr);
  EXPECT_EQ(*map.find("alpha"), 1);
  EXPECT_EQ(*map.find("beta"), 2);
  EXPECT_EQ(map.find("gamma"), nullptr);
}

TEST(UnorderedMapTest, IteratesEveryEntry) {
  base::UnorderedMap<i32, i32> map;
  for (i32 i = 0; i < 32; ++i) map.insert(i, i * 3);

  i32 seen = 0;
  i64 key_sum = 0;
  for (auto entry : map) {
    EXPECT_EQ(entry.value, entry.key * 3);
    key_sum += entry.key;
    ++seen;
  }

  EXPECT_EQ(seen, 32);
  EXPECT_EQ(key_sum, 31 * 32 / 2);
}

TEST(UnorderedMapTest, IteratesWhenConst) {
  base::UnorderedMap<i32, i32> map;
  map.insert(4, 40);
  const auto& const_map = map;

  mem_size seen = 0;
  for (auto entry : const_map) {
    EXPECT_EQ(entry.key, 4);
    EXPECT_EQ(entry.value, 40);
    ++seen;
  }
  EXPECT_EQ(seen, 1u);
}

TEST(UnorderedMapTest, CopyIsIndependent) {
  base::UnorderedMap<i32, i32> map;
  map.insert(1, 10);

  base::UnorderedMap<i32, i32> copy = map;
  copy[1] = 11;
  copy.insert(2, 20);

  EXPECT_EQ(*map.find(1), 10);
  EXPECT_EQ(map.size(), 1u);
  EXPECT_EQ(*copy.find(1), 11);
  EXPECT_EQ(copy.size(), 2u);
}

TEST(UnorderedMapTest, MoveLeavesTheSourceEmpty) {
  base::UnorderedMap<i32, i32> map;
  map.insert(1, 10);

  base::UnorderedMap<i32, i32> moved = base::move(map);

  EXPECT_EQ(*moved.find(1), 10);
  EXPECT_TRUE(map.empty());
}

TEST(UnorderedMapTest, Emplace) {
  base::UnorderedMap<i32, base::String> map;

  auto result = map.emplace(1, "value");

  EXPECT_TRUE(result.second);
  ASSERT_NE(map.find(1), nullptr);
  EXPECT_EQ(*map.find(1), "value");
}

TEST(UnorderedMapTest, TryEmplaceLeavesAnExistingKeyAlone) {
  base::UnorderedMap<i32, base::String> map;
  map.try_emplace(1, "first");

  auto again = map.try_emplace(1, "second");

  EXPECT_FALSE(again.second);
  EXPECT_EQ(*map.find(1), "first");
}

TEST(UnorderedMapInitTest, BraceInitialization) {
  base::UnorderedMap<base::String, base::String> map = {
      {"game", "Game"},
      {"debug", "Debug"},
  };

  EXPECT_EQ(map.size(), 2u);
  ASSERT_NE(map.find("game"), nullptr);
  EXPECT_EQ(*map.find("game"), "Game");
}

TEST(UnorderedMapInitTest, BraceInitializationKeepsTheFirstOfARepeatedKey) {
  base::UnorderedMap<i32, i32> map = {{1, 10}, {1, 20}};

  EXPECT_EQ(map.size(), 1u);
  EXPECT_EQ(*map.find(1), 10);
}

TEST(UnorderedMapTest, ReserveRoundsToAPowerOfTwoAboveTheRequest) {
  base::UnorderedMap<i32, i32> map;
  map.reserve(1000);
  // 1000 elements at a 0.75 load factor need 1334 buckets, so 2048.
  EXPECT_EQ(map.bucket_count(), 2048u);
  // Nothing rehashes while the reservation holds.
  for (i32 i = 0; i < 1000; ++i)
    map.insert(i, i);
  EXPECT_EQ(map.bucket_count(), 2048u);
  EXPECT_EQ(map.size(), 1000u);
}

TEST(UnorderedMapTest, ReserveBelowTheFloorStillGivesSixteenBuckets) {
  base::UnorderedMap<i32, i32> map;
  map.reserve(1);
  EXPECT_EQ(map.bucket_count(), 16u);
}

// reserve used to size the slot array with an unchecked multiply, and to round
// up with `while (target < required) target *= 2` -- a loop that wraps to zero
// and spins forever for a request this large.
TEST(UnorderedMapOverflowDeathTest, ReserveOfAnUnrepresentableCount) {
  // The alias keeps the template argument comma out of the macro argument list.
  using IntMap = base::UnorderedMap<i32, i32>;
  volatile mem_size count = base::MinMax<mem_size>::max() / 2;
  EXPECT_DEATH(
      {
        IntMap map;
        map.reserve(count);
      },
      "overflows");
}

}  // namespace
