
#include "lock_free_ordered_concurrent_hashmap.h"
#include <gtest/gtest.h>


class OrderedLockFreeHashMapTest : public ::testing::Test {
 protected:
  base::OrderedLockFreeHashMap<int, std::string> map;

  OrderedLockFreeHashMapTest()
      : map(10) {}  // Assuming 10 buckets for simplicity

  virtual ~OrderedLockFreeHashMapTest() {}
};

TEST_F(OrderedLockFreeHashMapTest, InsertionOrder) {
  map.insert(1, "First");
  map.insert(2, "Second");
  map.insert(3, "Third");

  auto node = map.orderHead.load();
  EXPECT_EQ(node->keyValue.second, "First");
  node = node->orderNext;
  EXPECT_EQ(node->keyValue.second, "Second");
  node = node->orderNext;
  EXPECT_EQ(node->keyValue.second, "Third");
}

TEST_F(OrderedLockFreeHashMapTest, FindElement) {
  std::string value;
  map.insert(1, "First");
  bool found = map.find(1, value);
  EXPECT_TRUE(found);
  EXPECT_EQ(value, "First");
}

TEST_F(OrderedLockFreeHashMapTest, RemoveAndOrder) {
  map.insert(1, "First");
  map.insert(2, "Second");
  map.insert(3, "Third");

  map.remove(2);  // Remove the middle element

  auto node = map.orderHead.load();
  EXPECT_EQ(node->keyValue.second, "First");
  node = node->orderNext;
  EXPECT_EQ(node->keyValue.second, "Third");
  EXPECT_EQ(node->orderNext, nullptr);
}

class LargeInsertionTest : public ::testing::Test {
 protected:
  base::OrderedLockFreeHashMap<int, std::string> map;

  LargeInsertionTest() : map(100) {}  // Using 100 buckets

  virtual ~LargeInsertionTest() {}
};

// Test for inserting and verifying a large number of elements
TEST_F(LargeInsertionTest, VerifyOrderAfterManyInsertions) {
  int numElements = 10000;  // For example, insert 10,000 elements
  std::vector<int> insertedOrder;

  // Insert elements
  for (int i = 0; i < numElements; ++i) {
    std::string value = "Value" + std::to_string(i);
    map.insert(i, std::move(value));
    insertedOrder.push_back(i);
  }

  // Verify the order
  auto node = map.orderHead.load();
  for (int i = 0; i < numElements; ++i) {
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->keyValue.first, insertedOrder[i]);
    node = node->orderNext;
  }
}