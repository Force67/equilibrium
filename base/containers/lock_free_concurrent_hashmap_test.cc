#include "lock_free_concurrent_hashmap.h"
#include <gtest/gtest.h>
#include <thread>
#include <vector>

using Test_HashMap_Type = base::LockFreeHashMap<int, int>;

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

void concurrentInsert(Test_HashMap_Type* hashMap, int threadId, int numInserts) {
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
void concurrentRemove(Test_HashMap_Type* hashMap, int threadId, int numRemoves) {
  for (int i = 0; i < numRemoves; ++i) {
    hashMap->remove(threadId * 1000 + i);
  }
}

void concurrentFind(Test_HashMap_Type* hashMap,
                    int threadId,
                    int numFinds,
                    base::Atomic<int>& foundCount) {
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
  base::Atomic<int> foundCount(0);

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
  int numOpsPerThread = 300;  // Ensure this is a multiple of 3 for insert, remove, find
  std::vector<std::thread> threads;

  for (int i = 0; i < numThreads; ++i) {
    threads.emplace_back(mixedOperations, hashMap, i, numOpsPerThread);
  }

  for (auto& thread : threads) {
    thread.join();
  }
}

// Stress tests targeting the dangerous concurrent paths in remove():
//   - Many threads removing different keys that hash to the same bucket
//     (linked-list pointer fix-ups race).
//   - Concurrent remove + find on the same bucket (documented as unsafe;
//     tested directly).

TEST(LockFreeHashMapStress, ConcurrentRemoveSameBucket) {
  // 8 buckets, many keys per bucket → forces remove threads onto the same
  // linked list. Repeat several rounds because the race is timing-dependent.
  constexpr int kRounds = 50;
  constexpr int kBuckets = 8;
  constexpr int kThreads = 8;
  constexpr int kKeysPerThread = 64;

  for (int round = 0; round < kRounds; ++round) {
    base::LockFreeHashMap<int, int> map(kBuckets);
    // Each thread owns a disjoint key range, but every bucket holds keys
    // from every thread (since key % kBuckets == j for any thread when
    // j ∈ [0, kBuckets)).
    const int total = kThreads * kKeysPerThread;
    for (int k = 0; k < total; ++k)
      map.insert(k, k);

    std::vector<std::thread> ts;
    ts.reserve(kThreads);
    for (int t = 0; t < kThreads; ++t) {
      ts.emplace_back([&, t] {
        for (int i = 0; i < kKeysPerThread; ++i) {
          map.remove(t * kKeysPerThread + i);
        }
      });
    }
    for (auto& th : ts)
      th.join();

    // Every key should be gone.
    for (int k = 0; k < total; ++k) {
      int v;
      ASSERT_FALSE(map.find(k, v))
          << "round=" << round << " key=" << k << " survived removal";
    }
  }
}

TEST(LockFreeHashMapStress, InsertFindHammer) {
  // Many threads inserting and finding into a small bucket count. Pure
  // insert+find is the safe pattern (the comment block on remove() makes
  // explicit that find races only with remove). This test should always pass.
  constexpr int kBuckets = 16;
  constexpr int kThreads = 8;
  constexpr int kOps = 5000;
  base::LockFreeHashMap<int, int> map(kBuckets);

  base::Atomic<bool> go{false};
  std::vector<std::thread> ts;
  for (int t = 0; t < kThreads / 2; ++t) {
    ts.emplace_back([&, t] {
      while (!go) {
      }
      for (int i = 0; i < kOps; ++i) {
        map.insert(t * kOps + i, t * kOps + i);
      }
    });
  }
  base::Atomic<int> total_found{0};
  for (int t = 0; t < kThreads / 2; ++t) {
    ts.emplace_back([&] {
      while (!go) {
      }
      for (int i = 0; i < kOps; ++i) {
        int v;
        if (map.find(i, v))
          total_found.fetch_add(1, base::memory_order_relaxed);
      }
    });
  }
  go.store(true);
  for (auto& th : ts)
    th.join();
  // No specific count check — we just want no crashes / no data races.
  SUCCEED();
}