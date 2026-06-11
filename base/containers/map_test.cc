#include <gtest/gtest.h>

#include <string>

#include "map.h"

namespace base {
namespace {
class MapTest : public ::testing::Test {
 protected:
  base::Map<int, std::string> map;
};

TEST_F(MapTest, InsertAndContains) {
  map.Insert(1, "one");
  EXPECT_TRUE(map.Contains(1));
  EXPECT_FALSE(map.Contains(2));
}

TEST_F(MapTest, DuplicateInsert) {
  EXPECT_TRUE(map.Insert(1, "one"));
  EXPECT_FALSE(map.Insert(1, "uno"));
  EXPECT_EQ(*map.Find(1), "one");
  EXPECT_EQ(map.size(), 1u);
}

TEST_F(MapTest, Erase) {
  map.Insert(1, "one");
  map.Erase(1);
  EXPECT_FALSE(map.Contains(1));
  EXPECT_EQ(map.size(), 0u);
}

TEST_F(MapTest, EraseNonExistentKey) {
  EXPECT_FALSE(map.Erase(2));
  EXPECT_EQ(map.size(), 0u);
}

TEST_F(MapTest, IterationIsInKeyOrder) {
  map.Insert(3, "three");
  map.Insert(1, "one");
  map.Insert(2, "two");

  int expected = 1;
  for (const auto& entry : map) {
    EXPECT_EQ(entry.first, expected);
    ++expected;
  }
  EXPECT_EQ(expected, 4);
}

TEST_F(MapTest, EmptyIteration) {
  EXPECT_EQ(map.begin(), map.end());
}

TEST_F(MapTest, FindReturnsIterator) {
  map.Insert(7, "seven");
  auto it = map.find(7);
  ASSERT_NE(it, map.end());
  EXPECT_EQ(it->first, 7);
  EXPECT_EQ(it->second, "seven");
  EXPECT_EQ(map.find(8), map.end());
}

TEST_F(MapTest, SubscriptInsertsAndUpdates) {
  map[5] = "five";
  EXPECT_EQ(map.size(), 1u);
  EXPECT_EQ(*map.Find(5), "five");
  map[5] = "cinq";
  EXPECT_EQ(map.size(), 1u);
  EXPECT_EQ(*map.Find(5), "cinq");
}

TEST_F(MapTest, EmplaceReportsInsertion) {
  auto first = map.emplace(9, "nine");
  EXPECT_TRUE(first.second);
  EXPECT_EQ(first.first->second, "nine");

  auto second = map.emplace(9, "neuf");
  EXPECT_FALSE(second.second);
  EXPECT_EQ(second.first->second, "nine");
}

TEST_F(MapTest, EraseIteratorReturnsSuccessor) {
  map.Insert(1, "one");
  map.Insert(2, "two");
  map.Insert(3, "three");

  auto it = map.find(2);
  it = map.erase(it);
  ASSERT_NE(it, map.end());
  EXPECT_EQ(it->first, 3);
  EXPECT_EQ(map.size(), 2u);
  EXPECT_FALSE(map.contains(2));

  it = map.erase(map.find(3));
  EXPECT_EQ(it, map.end());
}

TEST_F(MapTest, MutationThroughIterator) {
  map.Insert(1, "one");
  map.find(1)->second = "uno";
  EXPECT_EQ(*map.Find(1), "uno");
}

TEST_F(MapTest, ClearEmptiesTheMap) {
  map.Insert(1, "one");
  map.Insert(2, "two");
  map.clear();
  EXPECT_TRUE(map.empty());
  EXPECT_EQ(map.begin(), map.end());
}

TEST_F(MapTest, IterationSurvivesRebalancing) {
  for (int i = 0; i < 100; ++i) {
    map.Insert(i, std::to_string(i));
  }
  for (int i = 0; i < 100; i += 2) {
    EXPECT_TRUE(map.erase(i));
  }
  int expected = 1;
  for (const auto& entry : map) {
    EXPECT_EQ(entry.first, expected);
    expected += 2;
  }
  EXPECT_EQ(expected, 101);
}
}  // namespace
}  // namespace base
