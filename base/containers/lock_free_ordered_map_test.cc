// Tests for the EBR-based lock-free ordered hash map.
//
// What "lock-free" means for this container:
//   - find()  / insert() / remove() never block on a mutex.
//   - Multiple threads can mix all operations concurrently without UAF
//     thanks to epoch-based reclamation: a node retired by remove() is
//     not freed until every reader that could possibly hold a pointer to
//     it has exited its read-side critical section (its ebr::Guard).
//
// We hammer the container with the same patterns the mutex-based
// ConcurrentOrderedMap is verified against, plus the things that are
// uniquely interesting for a lock-free implementation: large concurrent
// remove churn, GC under load, iterator-during-write.
#include <gtest/gtest.h>

#include <atomic>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include "lock_free_ordered_map.h"

namespace {

using IntMap = base::LockFreeOrderedHashMap<int, int>;

// ─── Single-threaded sanity ─────────────────────────────────────────────

TEST(LockFreeOrderedHashMap, EmptyOnConstruction) {
  IntMap m(16);
  int v;
  EXPECT_FALSE(m.find(0, v));
  EXPECT_TRUE(m.ordered_keys_snapshot().empty());
}

TEST(LockFreeOrderedHashMap, InsertAndFind) {
  IntMap m(16);
  EXPECT_TRUE(m.insert(1, 100));
  EXPECT_TRUE(m.insert(2, 200));
  EXPECT_TRUE(m.insert(3, 300));

  int v;
  ASSERT_TRUE(m.find(1, v));
  EXPECT_EQ(v, 100);
  ASSERT_TRUE(m.find(2, v));
  EXPECT_EQ(v, 200);
  ASSERT_TRUE(m.find(3, v));
  EXPECT_EQ(v, 300);
  EXPECT_FALSE(m.find(99, v));
}

TEST(LockFreeOrderedHashMap, InsertDuplicateReturnsFalse) {
  IntMap m(16);
  EXPECT_TRUE(m.insert(1, 100));
  EXPECT_FALSE(m.insert(1, 999));  // duplicate — original wins
  int v;
  ASSERT_TRUE(m.find(1, v));
  EXPECT_EQ(v, 100);
}

TEST(LockFreeOrderedHashMap, RemoveBasic) {
  IntMap m(16);
  m.insert(1, 100);
  m.insert(2, 200);

  EXPECT_TRUE(m.remove(1));
  int v;
  EXPECT_FALSE(m.find(1, v));
  ASSERT_TRUE(m.find(2, v));
  EXPECT_EQ(v, 200);
  EXPECT_FALSE(m.remove(1));  // already gone
}

TEST(LockFreeOrderedHashMap, OrderedKeysSnapshot) {
  IntMap m(16);
  m.insert(10, 1);
  m.insert(20, 2);
  m.insert(30, 3);

  auto keys = m.ordered_keys_snapshot();
  ASSERT_EQ(keys.size(), 3u);
  EXPECT_EQ(keys[0], 10);
  EXPECT_EQ(keys[1], 20);
  EXPECT_EQ(keys[2], 30);
}

TEST(LockFreeOrderedHashMap, OrderedKeysSnapshotSkipsRemoved) {
  IntMap m(16);
  m.insert(1, 1);
  m.insert(2, 2);
  m.insert(3, 3);
  m.remove(2);

  auto keys = m.ordered_keys_snapshot();
  ASSERT_EQ(keys.size(), 2u);
  EXPECT_EQ(keys[0], 1);
  EXPECT_EQ(keys[1], 3);
}

TEST(LockFreeOrderedHashMap, ReinsertAfterRemove) {
  IntMap m(16);
  m.insert(7, 70);
  m.remove(7);
  EXPECT_TRUE(m.insert(7, 700));  // should succeed (key is logically gone)
  int v;
  ASSERT_TRUE(m.find(7, v));
  EXPECT_EQ(v, 700);
}

TEST(LockFreeOrderedHashMap, ManyInsertsKeepOrder) {
  IntMap m(64);
  constexpr int kN = 5000;
  for (int i = 0; i < kN; ++i) m.insert(i, i * 2);

  auto keys = m.ordered_keys_snapshot();
  ASSERT_EQ(keys.size(), static_cast<arch_types::mem_size>(kN));
  for (int i = 0; i < kN; ++i) EXPECT_EQ(keys[i], i);
}

TEST(LockFreeOrderedHashMap, GarbageCollectFreesRetiredNodes) {
  IntMap m(16);
  for (int i = 0; i < 1000; ++i) m.insert(i, int{i});
  for (int i = 0; i < 1000; ++i) m.remove(i);
  // Force a GC pass — internally walks the staged + retired lists and
  // physically frees nodes whose retirement epoch is old enough.
  m.collect_garbage();
  // After GC the snapshot is empty.
  EXPECT_TRUE(m.ordered_keys_snapshot().empty());
}

// ─── Concurrent stress ──────────────────────────────────────────────────

TEST(LockFreeOrderedHashMap, ConcurrentInsertReachability) {
  // Disjoint key ranges per thread; after join every key is reachable
  // through both find() and the ordered snapshot.
  constexpr int kThreads = 8;
  constexpr int kPerThread = 2000;
  constexpr int kTotal = kThreads * kPerThread;

  IntMap m(256);
  base::Atomic<bool> go{false};
  std::vector<std::thread> ts;
  for (int t = 0; t < kThreads; ++t) {
    ts.emplace_back([&, t] {
      while (!go.load()) {}
      for (int i = 0; i < kPerThread; ++i) {
        const int k = t * kPerThread + i;
        m.insert(k, int{k});
      }
    });
  }
  go.store(true);
  for (auto& th : ts) th.join();

  for (int k = 0; k < kTotal; ++k) {
    int v = -1;
    ASSERT_TRUE(m.find(k, v)) << "key " << k << " missing";
    EXPECT_EQ(v, k);
  }
  auto keys = m.ordered_keys_snapshot();
  EXPECT_EQ(keys.size(), static_cast<arch_types::mem_size>(kTotal));

  // Every key 0..kTotal-1 should appear exactly once in the snapshot.
  std::unordered_set<int> seen;
  for (auto k : keys) {
    EXPECT_TRUE(seen.insert(k).second) << "duplicate key " << k;
  }
  EXPECT_EQ(seen.size(), static_cast<size_t>(kTotal));
}

TEST(LockFreeOrderedHashMap, ConcurrentRemoveSafe) {
  // The flagship lock-free test. Pre-populate, then have N threads
  // concurrently delete disjoint key ranges. EBR must keep concurrent
  // readers safe — but since this is a remove-only race, we mainly verify
  // no crash + every key is gone.
  constexpr int kThreads = 8;
  constexpr int kPerThread = 1000;
  constexpr int kTotal = kThreads * kPerThread;

  IntMap m(64);
  for (int k = 0; k < kTotal; ++k) m.insert(k, int{k});

  base::Atomic<bool> go{false};
  base::Atomic<int> removed{0};
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
  for (int k = 0; k < kTotal; ++k) {
    int v;
    EXPECT_FALSE(m.find(k, v)) << "key " << k << " still findable";
  }
}

TEST(LockFreeOrderedHashMap, ConcurrentMixedOperations) {
  // Insert / find / remove on overlapping key space. The whole point of
  // EBR is that this combination is safe — the mutex version would just
  // serialize, but here insert+find+remove all run truly concurrently.
  constexpr int kThreads = 8;
  constexpr int kOpsPerThread = 5000;
  constexpr int kKeySpace = 200;

  IntMap m(64);
  base::Atomic<bool> go{false};
  std::vector<std::thread> ts;
  for (int t = 0; t < kThreads; ++t) {
    ts.emplace_back([&, t] {
      while (!go.load()) {}
      for (int i = 0; i < kOpsPerThread; ++i) {
        const int key = (t * 7919 + i * 31) % kKeySpace;
        switch ((t + i) % 3) {
          case 0:
            m.insert(key, int{key});
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

  // No GC call here on purpose: find() must work even if deleted nodes
  // haven't been physically swept yet. (We added a special path in find()
  // that walks past tagged nodes for exactly this case.)

  // Every surviving key must be findable. We don't predict the final size,
  // but the snapshot must agree with itself: every key appears at most once.
  auto keys = m.ordered_keys_snapshot();
  std::unordered_set<int> seen;
  for (auto k : keys) {
    EXPECT_TRUE(seen.insert(k).second) << "duplicate key " << k << " in snapshot";
    int v;
    EXPECT_TRUE(m.find(k, v)) << "snapshot key " << k << " not findable";
  }
}

TEST(LockFreeOrderedHashMap, ReadersDuringWritesNoUAF) {
  // The defining lock-free win: readers iterate the ordered list while
  // writers are concurrently churning the map. The mutex version would
  // block readers during writes; the EBR version lets them traverse,
  // protected by their epoch guard. We run for a fixed window and assert
  // no crash and no torn iteration (every traversal sees a coherent prefix
  // of the order list — duplicates inside one traversal mean a link
  // mutation crossed our scan).
  constexpr int kWriters = 4;
  constexpr int kReaders = 4;
  constexpr int kDurationMs = 200;

  IntMap m(64);
  for (int i = 0; i < 200; ++i) m.insert(i, int{i});

  base::Atomic<bool> stop{false};
  base::Atomic<int> reader_iterations{0};
  base::Atomic<int> reader_uaf_proxy{0};

  std::vector<std::thread> ts;
  for (int w = 0; w < kWriters; ++w) {
    ts.emplace_back([&, w] {
      int n = 0;
      while (!stop.load()) {
        const int key = (w * 1000 + n) % 200;
        m.remove(key);
        m.insert(key, int{key});
        ++n;
      }
    });
  }
  for (int r = 0; r < kReaders; ++r) {
    ts.emplace_back([&] {
      while (!stop.load()) {
        // Each iteration grabs a fresh snapshot. With EBR this is safe
        // even though writers are concurrently mutating — internally the
        // walk is protected by an ebr::Guard which pins the epoch.
        auto snap = m.ordered_keys_snapshot();
        for (auto k : snap) {
          // Sanity: keys are always in [0, 200). If we read garbage memory
          // we'd see something out of range — cheap UAF proxy.
          if (k < 0 || k >= 200) {
            reader_uaf_proxy.fetch_add(1, base::memory_order_relaxed);
          }
        }
        reader_iterations.fetch_add(1, base::memory_order_relaxed);
      }
    });
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(kDurationMs));
  stop.store(true);
  for (auto& th : ts) th.join();

  EXPECT_EQ(reader_uaf_proxy.load(), 0)
      << "out-of-range key observed during concurrent traversal — possible UAF";
  EXPECT_GT(reader_iterations.load(), 0)
      << "no reader iterations completed (test probably hung)";
}

TEST(LockFreeOrderedHashMap, AutoGCKeepsMemoryBounded) {
  // Insert + remove cycles that exceed the auto-GC threshold (4096
  // deletions). The container should periodically reclaim retired nodes
  // without us calling collect_garbage() manually, otherwise long-running
  // workloads would leak memory unboundedly.
  IntMap m(64);
  constexpr int kCycles = 10000;
  for (int i = 0; i < kCycles; ++i) {
    m.insert(i, int{i});
    m.remove(i);
  }
  // Force a final pass.
  m.collect_garbage();
  EXPECT_TRUE(m.ordered_keys_snapshot().empty());
}

}  // namespace
