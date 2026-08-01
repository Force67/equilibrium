#include <gtest/gtest.h>

#include <string>

#include "map.h"
#include "set.h"
#include "vector.h"

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

TEST(MapPairKeyTest, OrdersLexicographically) {
  base::Map<base::Pair<i32, i32>, i32> map;
  map[{1, 2}] = 12;
  map[{1, 1}] = 11;
  map[{0, 9}] = 9;

  EXPECT_EQ(map.size(), 3u);
  EXPECT_TRUE(map.contains(base::Pair<i32, i32>{1, 1}));

  base::Vector<i32> in_order;
  for (auto it = map.begin(); it != map.end(); ++it) in_order.push_back(it->second);

  ASSERT_EQ(in_order.size(), 3u);
  EXPECT_EQ(in_order[0], 9);
  EXPECT_EQ(in_order[1], 11);
  EXPECT_EQ(in_order[2], 12);
}

TEST(PairTest, ComparisonOperators) {
  base::Pair<i32, i32> a{1, 2};
  base::Pair<i32, i32> b{1, 3};

  EXPECT_TRUE(a == a);
  EXPECT_TRUE(a != b);
  EXPECT_TRUE(a < b);
  EXPECT_TRUE(b > a);
  EXPECT_TRUE(a <= a);
  EXPECT_TRUE(a >= a);
}


// Regression: Map and Set used to fall back on the implicit member-wise copy,
// which shared the tree's nodes and sentinel between both objects and then
// double-freed them.
TEST(MapValueSemanticsTest, CopyIsDeepAndIndependent) {
  base::Map<i32, i32> map;
  map[1] = 10;
  map[2] = 20;

  base::Map<i32, i32> copy = map;
  copy[1] = 11;
  copy[3] = 30;

  EXPECT_EQ(map.size(), 2u);
  EXPECT_EQ(map.find(1)->second, 10);
  EXPECT_FALSE(map.contains(3));
  EXPECT_EQ(copy.size(), 3u);
  EXPECT_EQ(copy.find(1)->second, 11);
}

TEST(MapValueSemanticsTest, CopyAssignmentReplacesTheTarget) {
  base::Map<i32, i32> map;
  map[1] = 10;
  base::Map<i32, i32> other;
  other[9] = 90;

  other = map;

  EXPECT_EQ(other.size(), 1u);
  EXPECT_TRUE(other.contains(1));
  EXPECT_FALSE(other.contains(9));
}

TEST(MapValueSemanticsTest, MoveLeavesTheSourceEmptyAndUsable) {
  base::Map<i32, i32> map;
  map[1] = 10;

  base::Map<i32, i32> moved = base::move(map);

  EXPECT_EQ(moved.size(), 1u);
  EXPECT_EQ(moved.find(1)->second, 10);
  EXPECT_EQ(map.size(), 0u);
  EXPECT_TRUE(map.empty());

  map[5] = 50;  // still usable after the move
  EXPECT_EQ(map.size(), 1u);
}

TEST(MapValueSemanticsTest, MoveAssignmentReplacesTheTarget) {
  base::Map<i32, i32> map;
  map[1] = 10;
  base::Map<i32, i32> other;
  other[9] = 90;

  other = base::move(map);

  EXPECT_EQ(other.size(), 1u);
  EXPECT_TRUE(other.contains(1));
  EXPECT_TRUE(map.empty());
}

TEST(SetValueSemanticsTest, CopyIsDeepAndIndependent) {
  base::Set<i32> set;
  set.insert(1);

  base::Set<i32> copy = set;
  copy.insert(2);

  EXPECT_EQ(set.size(), 1u);
  EXPECT_FALSE(set.contains(2));
  EXPECT_EQ(copy.size(), 2u);
}

TEST(SetValueSemanticsTest, MoveLeavesTheSourceEmpty) {
  base::Set<i32> set;
  set.insert(1);

  base::Set<i32> moved = base::move(set);

  EXPECT_EQ(moved.size(), 1u);
  EXPECT_TRUE(moved.contains(1));
  EXPECT_TRUE(set.empty());
}

}  // namespace base
