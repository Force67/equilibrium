#include "lock_free_concurrent_hashmap.h"
#include <gtest/gtest.h>
#include <thread>
#include <vector>

using Test_HashMap_Type = base::LockFreeHashMap<int, int>;
#if 0
class LockFreeHashMapTest : public ::testing::Test {
 protected:
  Test_HashMap_Type* hashMap;

  void SetUp() override {
    hashMap = new Test_HashMap_Type(100);  // Initialize with a suitable size
  }

  void TearDown() override { delete hashMap; }
};

TEST_F(LockFreeHashMapTest, SingleThreadedInsertFind) {
  hashMap->insert(1, 100);
  int value;
  ASSERT_TRUE(hashMap->find(1, value));
  ASSERT_EQ(value, 100);
}

TEST_F(LockFreeHashMapTest, SingleThreadedRemove) {
  hashMap->insert(2, 200);
  ASSERT_TRUE(hashMap->remove(2));
  int value;
  ASSERT_FALSE(hashMap->find(2, value));
}

void concurrentInsert(Test_HashMap_Type* hashMap,
                      int threadId,
                      int numInserts) {
  for (int i = 0; i < numInserts; ++i) {
    hashMap->insert(threadId * 1000 + i, i);
  }
}

TEST_F(LockFreeHashMapTest, MultiThreadedInsert) {
  int numThreads = 10;
  int numInsertsPerThread = 100;
  std::vector<std::thread> threads;

  for (int i = 0; i < numThreads; ++i) {
    threads.emplace_back(concurrentInsert, hashMap, i, numInsertsPerThread);
  }

  for (auto& thread : threads) {
    thread.join();
  }

  // Check if all values are inserted correctly
  for (int i = 0; i < numThreads; ++i) {
    for (int j = 0; j < numInsertsPerThread; ++j) {
      int value;
      ASSERT_TRUE(hashMap->find(i * 1000 + j, value));
      ASSERT_EQ(value, j);
    }
  }
}

// Additional functions for concurrent operations
void concurrentRemove(Test_HashMap_Type* hashMap,
                      int threadId,
                      int numRemoves) {
  for (int i = 0; i < numRemoves; ++i) {
    hashMap->remove(threadId * 1000 + i);
  }
}

void concurrentFind(Test_HashMap_Type* hashMap,
                    int threadId,
                    int numFinds,
                    std::atomic<int>& foundCount) {
  for (int i = 0; i < numFinds; ++i) {
    int value;
    if (hashMap->find(threadId * 1000 + i, value)) {
      foundCount.fetch_add(1);
    }
  }
}

void mixedOperations(Test_HashMap_Type* hashMap, int threadId, int numOps) {
  for (int i = 0; i < numOps; ++i) {
    switch (i % 3) {
      case 0:
        hashMap->insert(threadId * 1000 + i, i);
        break;
      case 1:
        hashMap->remove(threadId * 1000 + i);
        break;
      case 2:
        int value;
        hashMap->find(threadId * 1000 + i, value);
        break;
    }
  }
}

// Test for concurrent remove
TEST_F(LockFreeHashMapTest, MultiThreadedRemove) {
  int numThreads = 10;
  int numRemovesPerThread = 100;
  std::vector<std::thread> threads;

  // First insert some values
  for (int i = 0; i < numThreads; ++i) {
    concurrentInsert(hashMap, i, numRemovesPerThread);
  }

  // Now remove them concurrently
  for (int i = 0; i < numThreads; ++i) {
    threads.emplace_back(concurrentRemove, hashMap, i, numRemovesPerThread);
  }

  for (auto& thread : threads) {
    thread.join();
  }

  // Verify that all values are removed
  for (int i = 0; i < numThreads; ++i) {
    for (int j = 0; j < numRemovesPerThread; ++j) {
      int value;
      ASSERT_FALSE(hashMap->find(i * 1000 + j, value));
    }
  }
}

// Test for concurrent find
TEST_F(LockFreeHashMapTest, MultiThreadedFind) {
  int numThreads = 10;
  int numFindsPerThread = 100;
  std::vector<std::thread> threads;
  std::atomic<int> foundCount(0);

  // First insert some values
  for (int i = 0; i < numThreads; ++i) {
    concurrentInsert(hashMap, i, numFindsPerThread);
  }

  // Now search for them concurrently
  for (int i = 0; i < numThreads; ++i) {
    threads.emplace_back(concurrentFind, hashMap, i, numFindsPerThread,
                         std::ref(foundCount));
  }

  for (auto& thread : threads) {
    thread.join();
  }

  // Verify that all inserted values are found
  ASSERT_EQ(foundCount.load(), numThreads * numFindsPerThread);
}

// Test for mixed operations
TEST_F(LockFreeHashMapTest, MixedOperations) {
  int numThreads = 10;
  int numOpsPerThread =
      300;  // Ensure this is a multiple of 3 for insert, remove, find
  std::vector<std::thread> threads;

  for (int i = 0; i < numThreads; ++i) {
    threads.emplace_back(mixedOperations, hashMap, i, numOpsPerThread);
  }

  for (auto& thread : threads) {
    thread.join();
  }
}
#endif