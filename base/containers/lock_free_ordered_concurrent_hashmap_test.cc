
#include "lock_free_ordered_concurrent_hashmap.h"
#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

class OrderedLockFreeHashMapTest : public ::testing::Test {
 protected:
  base::OrderedLockFreeHashMap<int, std::string> map;

  OrderedLockFreeHashMapTest() : map(10) {}  // Assuming 10 buckets for simplicity

  virtual ~OrderedLockFreeHashMapTest() {}
};

TEST_F(OrderedLockFreeHashMapTest, InsertionOrder) {
  map.insert(1, "First");
  map.insert(2, "Second");
  map.insert(3, "Third");

  auto node = map.orderHead.load();
  EXPECT_EQ(node->keyValue.second, "First");
  node = node->orderNext.load();
  EXPECT_EQ(node->keyValue.second, "Second");
  node = node->orderNext.load();
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
  node = node->orderNext.load();
  EXPECT_EQ(node->keyValue.second, "Third");
  EXPECT_EQ(node->orderNext.load(), nullptr);
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
    node = node->orderNext.load();
  }
}

// ─────────────────────────────────────────────────────────────────────────
// Stress: concurrent inserts. The original implementation does NOT splice
// new nodes into the ordered list atomically (orderNext/orderPrev are plain
// pointers written inside the CAS loop), so this should expose lost links,
// stale links, or simply fewer than N nodes reachable from orderHead.
// ─────────────────────────────────────────────────────────────────────────

TEST(OrderedLockFreeHashMapStress, ConcurrentInsertReachability) {
  constexpr int kThreads = 8;
  constexpr int kPerThread = 2000;
  constexpr int kTotal = kThreads * kPerThread;

  base::OrderedLockFreeHashMap<int, int> map(256);
  std::atomic<bool> go{false};
  std::vector<std::thread> ts;
  ts.reserve(kThreads);
  for (int t = 0; t < kThreads; ++t) {
    ts.emplace_back([&, t] {
      while (!go.load()) {}
      for (int i = 0; i < kPerThread; ++i) {
        const int k = t * kPerThread + i;
        map.insert(k, int{k});
      }
    });
  }
  go.store(true);
  for (auto& th : ts) th.join();

  // Every key must be findable through the bucket index. This is the
  // strongest invariant we can verify after concurrent inserts; the ordered
  // list is documented as unsafe under concurrent insert.
  for (int k = 0; k < kTotal; ++k) {
    int v = -1;
    ASSERT_TRUE(map.find(k, v)) << "key " << k << " missing after concurrent insert";
    ASSERT_EQ(v, k);
  }

  // Walk the ordered list forward and count reachable nodes. With the fixed
  // insert(), every node is reachable from orderHead after all writers join.
  int forward = 0;
  auto* node = map.orderHead.load();
  while (node && forward <= kTotal + 8) {
    ++forward;
    node = node->orderNext.load();
  }
  EXPECT_EQ(forward, kTotal)
      << "ordered forward walk reached " << forward << " of " << kTotal
      << " — orderNext links were corrupted by concurrent insert";

  // Also walk backward from orderTail and verify the same count.
  int backward = 0;
  node = map.orderTail.load();
  while (node && backward <= kTotal + 8) {
    ++backward;
    node = node->orderPrev.load();
  }
  EXPECT_EQ(backward, kTotal)
      << "ordered backward walk reached " << backward << " of " << kTotal;
}