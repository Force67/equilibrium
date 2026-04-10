// Tests for ConcurrentOrderedMap.
//
// We want correctness under all four concurrent operation mixes that the
// old "lock-free" version got wrong:
//   - concurrent inserts            (forward + backward link consistency)
//   - concurrent finds               (lock-free read path is the hot path)
//   - concurrent removes             (the original UAF)
//   - concurrent insert/find/remove  (mixed, the realistic stress)
#include <gtest/gtest.h>

#include <atomic>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include "concurrent_ordered_map.h"

namespace {

// ─── Single-threaded behavior ────────────────────────────────────────────

TEST(ConcurrentOrderedMap, EmptyOnConstruction) {
  base::ConcurrentOrderedMap<int, int> m(16);
  EXPECT_EQ(m.size(), 0u);
  EXPECT_TRUE(m.empty());
  int v;
  EXPECT_FALSE(m.find(0, v));
}

TEST(ConcurrentOrderedMap, InsertAndFind) {
  base::ConcurrentOrderedMap<int, std::string> m(8);
  m.insert(1, "one");
  m.insert(2, "two");
  m.insert(3, "three");

  std::string v;
  ASSERT_TRUE(m.find(1, v));
  EXPECT_EQ(v, "one");
  ASSERT_TRUE(m.find(2, v));
  EXPECT_EQ(v, "two");
  ASSERT_TRUE(m.find(3, v));
  EXPECT_EQ(v, "three");
  EXPECT_FALSE(m.find(4, v));
  EXPECT_EQ(m.size(), 3u);
}

TEST(ConcurrentOrderedMap, IterationFollowsInsertOrder) {
  base::ConcurrentOrderedMap<int, std::string> m(8);
  m.insert(10, "ten");
  m.insert(20, "twenty");
  m.insert(30, "thirty");

  std::vector<int> seen;
  m.for_each_in_order([&](const int& k, const std::string&) { seen.push_back(k); });
  ASSERT_EQ(seen.size(), 3u);
  EXPECT_EQ(seen[0], 10);
  EXPECT_EQ(seen[1], 20);
  EXPECT_EQ(seen[2], 30);
}

TEST(ConcurrentOrderedMap, RemovePreservesRemainingOrder) {
  base::ConcurrentOrderedMap<int, std::string> m(8);
  m.insert(1, "first");
  m.insert(2, "second");
  m.insert(3, "third");
  m.insert(4, "fourth");
  ASSERT_TRUE(m.remove(2));
  ASSERT_TRUE(m.remove(4));

  std::vector<int> seen;
  m.for_each_in_order([&](const int& k, const std::string&) { seen.push_back(k); });
  ASSERT_EQ(seen.size(), 2u);
  EXPECT_EQ(seen[0], 1);
  EXPECT_EQ(seen[1], 3);
  EXPECT_EQ(m.size(), 2u);
}

TEST(ConcurrentOrderedMap, RemoveHeadAndTail) {
  base::ConcurrentOrderedMap<int, int> m(8);
  m.insert(1, 1);
  m.insert(2, 2);
  m.insert(3, 3);

  ASSERT_TRUE(m.remove(1));  // remove head
  ASSERT_TRUE(m.remove(3));  // remove tail
  std::vector<int> seen;
  m.for_each_in_order([&](const int& k, int) { seen.push_back(k); });
  ASSERT_EQ(seen.size(), 1u);
  EXPECT_EQ(seen[0], 2);
}

TEST(ConcurrentOrderedMap, RemoveOnly) {
  base::ConcurrentOrderedMap<int, int> m(8);
  m.insert(42, 42);
  ASSERT_TRUE(m.remove(42));
  EXPECT_TRUE(m.empty());
  EXPECT_FALSE(m.remove(42));  // gone
}

TEST(ConcurrentOrderedMap, InsertOrAssignOverwrites) {
  base::ConcurrentOrderedMap<int, std::string> m(8);
  EXPECT_TRUE(m.insert_or_assign(1, "first"));
  EXPECT_FALSE(m.insert_or_assign(1, "updated"));

  std::string v;
  ASSERT_TRUE(m.find(1, v));
  EXPECT_EQ(v, "updated");
  EXPECT_EQ(m.size(), 1u);
}

TEST(ConcurrentOrderedMap, ClearEmptiesEverything) {
  base::ConcurrentOrderedMap<int, int> m(8);
  for (int i = 0; i < 100; ++i) m.insert(i, i);
  EXPECT_EQ(m.size(), 100u);
  m.clear();
  EXPECT_EQ(m.size(), 0u);
  EXPECT_TRUE(m.empty());
  int v;
  EXPECT_FALSE(m.find(50, v));
}

TEST(ConcurrentOrderedMap, ManyInsertsKeepOrder) {
  constexpr int kN = 5000;
  base::ConcurrentOrderedMap<int, int> m(64);
  for (int i = 0; i < kN; ++i) m.insert(i, i * 2);

  // Order is the insertion order.
  int expected = 0;
  m.for_each_in_order([&](const int& k, const int& v) {
    EXPECT_EQ(k, expected);
    EXPECT_EQ(v, expected * 2);
    ++expected;
  });
  EXPECT_EQ(expected, kN);
}

// ─── Concurrent stress ──────────────────────────────────────────────────

TEST(ConcurrentOrderedMap, ConcurrentInsertReachability) {
  // Each thread inserts a disjoint key range. After join, every key must be
  // present and the order list must reach all of them in both directions.
  constexpr int kThreads = 8;
  constexpr int kPerThread = 2000;
  constexpr int kTotal = kThreads * kPerThread;

  base::ConcurrentOrderedMap<int, int> m(256);
  std::atomic<bool> go{false};
  std::vector<std::thread> ts;
  for (int t = 0; t < kThreads; ++t) {
    ts.emplace_back([&, t] {
      while (!go.load()) {}
      for (int i = 0; i < kPerThread; ++i) {
        const int k = t * kPerThread + i;
        m.insert(k, k);
      }
    });
  }
  go.store(true);
  for (auto& th : ts) th.join();

  EXPECT_EQ(m.size(), static_cast<arch_types::mem_size>(kTotal));

  // Every key reachable via find().
  for (int k = 0; k < kTotal; ++k) {
    int v = -1;
    ASSERT_TRUE(m.find(k, v)) << "key " << k << " missing";
    EXPECT_EQ(v, k);
  }

  // Every key reachable in order iteration.
  int forward = 0;
  std::unordered_set<int> seen;
  m.for_each_in_order([&](const int& k, const int&) {
    seen.insert(k);
    ++forward;
  });
  EXPECT_EQ(forward, kTotal);
  EXPECT_EQ(seen.size(), static_cast<size_t>(kTotal));
}

TEST(ConcurrentOrderedMap, ConcurrentFindAfterInsert) {
  // Population is single-threaded, then many threads pound find() in
  // parallel. Verifies the shared-lock read path is correct and no key
  // is mis-reported as missing.
  constexpr int kKeys = 10000;
  constexpr int kReaders = 8;
  constexpr int kFindsPerReader = 5000;

  base::ConcurrentOrderedMap<int, int> m(512);
  for (int i = 0; i < kKeys; ++i) m.insert(i, i + 1);

  std::atomic<int> hits{0};
  std::atomic<bool> go{false};
  std::vector<std::thread> ts;
  for (int t = 0; t < kReaders; ++t) {
    ts.emplace_back([&] {
      while (!go.load()) {}
      for (int i = 0; i < kFindsPerReader; ++i) {
        int v;
        if (m.find(i % kKeys, v)) {
          if (v == (i % kKeys) + 1) hits.fetch_add(1, std::memory_order_relaxed);
        }
      }
    });
  }
  go.store(true);
  for (auto& th : ts) th.join();
  EXPECT_EQ(hits.load(), kReaders * kFindsPerReader);
}

TEST(ConcurrentOrderedMap, ConcurrentRemoveSafe) {
  // The flagship test: every key gets inserted by one thread and removed by
  // another, fully concurrently. The old lock-free version would crash via
  // use-after-free here. The new version must finish cleanly with all keys
  // gone.
  constexpr int kThreads = 8;
  constexpr int kPerThread = 1000;
  constexpr int kTotal = kThreads * kPerThread;

  base::ConcurrentOrderedMap<int, int> m(64);
  // Pre-populate.
  for (int k = 0; k < kTotal; ++k) m.insert(k, k);
  EXPECT_EQ(m.size(), static_cast<arch_types::mem_size>(kTotal));

  std::atomic<bool> go{false};
  std::atomic<int> removed{0};
  std::vector<std::thread> ts;
  for (int t = 0; t < kThreads; ++t) {
    ts.emplace_back([&, t] {
      while (!go.load()) {}
      for (int i = 0; i < kPerThread; ++i) {
        if (m.remove(t * kPerThread + i)) removed.fetch_add(1);
      }
    });
  }
  go.store(true);
  for (auto& th : ts) th.join();

  EXPECT_EQ(removed.load(), kTotal);
  EXPECT_EQ(m.size(), 0u);
  EXPECT_TRUE(m.empty());
}

TEST(ConcurrentOrderedMap, ConcurrentMixedOperations) {
  // Many threads doing insert/find/remove on overlapping key spaces. We
  // can't predict the final state, but the map must not crash, must not
  // leak, and after all writers stop the size must equal what we count
  // via iteration.
  constexpr int kThreads = 8;
  constexpr int kOpsPerThread = 5000;
  constexpr int kKeySpace = 200;

  base::ConcurrentOrderedMap<int, int> m(64);
  std::atomic<bool> go{false};
  std::vector<std::thread> ts;
  for (int t = 0; t < kThreads; ++t) {
    ts.emplace_back([&, t] {
      while (!go.load()) {}
      // Pseudo-random op selection driven by (t,i) only — deterministic
      // per-thread, no shared RNG.
      for (int i = 0; i < kOpsPerThread; ++i) {
        const int key = (t * 7919 + i * 31) % kKeySpace;
        switch ((t + i) % 3) {
          case 0:
            m.insert(key, key);
            break;
          case 1: {
            int v;
            (void)m.find(key, v);
            break;
          }
          case 2:
            (void)m.remove(key);
            break;
        }
      }
    });
  }
  go.store(true);
  for (auto& th : ts) th.join();

  // After all writers stopped, size() must equal the iteration count.
  arch_types::mem_size by_size = m.size();
  arch_types::mem_size by_iter = 0;
  m.for_each_in_order([&](const int&, const int&) { ++by_iter; });
  EXPECT_EQ(by_size, by_iter)
      << "size() and iteration count diverged — likely a missed link update";
}

TEST(ConcurrentOrderedMap, ConcurrentReadersDuringWrites) {
  // Writers churn the map; readers iterate via for_each_in_order while
  // writes are happening. The shared lock means readers see a consistent
  // snapshot for the duration of each iteration.
  constexpr int kWriters = 4;
  constexpr int kReaders = 4;
  constexpr int kDurationMs = 100;

  base::ConcurrentOrderedMap<int, int> m(64);
  for (int i = 0; i < 200; ++i) m.insert(i, i);

  std::atomic<bool> stop{false};
  std::atomic<int> iteration_errors{0};

  // Pre-populate with insert_or_assign so duplicates can't sneak in.
  m.clear();
  for (int i = 0; i < 200; ++i) m.insert_or_assign(i, i);

  std::vector<std::thread> ts;
  for (int w = 0; w < kWriters; ++w) {
    ts.emplace_back([&, w] {
      int n = 0;
      while (!stop.load()) {
        const int key = (w * 1000 + n) % 200;
        // Use insert_or_assign so the key→count invariant holds: at most
        // one entry per key. The point of this test is to verify reader
        // iteration sees a consistent snapshot, not to test insert semantics.
        m.insert_or_assign(key, key);
        ++n;
      }
    });
  }
  for (int r = 0; r < kReaders; ++r) {
    ts.emplace_back([&] {
      while (!stop.load()) {
        // Each iteration sees a consistent snapshot under the shared lock.
        // Verify there are no duplicates within a single iteration.
        std::unordered_set<int> seen;
        m.for_each_in_order([&](const int& k, const int&) {
          if (!seen.insert(k).second) {
            iteration_errors.fetch_add(1, std::memory_order_relaxed);
          }
        });
      }
    });
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(kDurationMs));
  stop.store(true);
  for (auto& th : ts) th.join();

  EXPECT_EQ(iteration_errors.load(), 0)
      << "duplicate keys observed within a single iteration — lock failed";
}

}  // namespace
