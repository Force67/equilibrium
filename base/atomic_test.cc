// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/atomic.h>

#include <thread>

namespace base {

// ---- scalar load/store/exchange -------------------------------------------

TEST(Atomic, DefaultCtorZero) {
  Atomic<i32> a;
  EXPECT_EQ(a.load(), 0);
}

TEST(Atomic, ValueCtor) {
  Atomic<i32> a(42);
  EXPECT_EQ(a.load(), 42);
}

TEST(Atomic, LoadStore) {
  Atomic<i32> a(0);
  a.store(123);
  EXPECT_EQ(a.load(), 123);
}

TEST(Atomic, LoadStoreAllMemoryOrders) {
  Atomic<i32> a(0);
  a.store(1, memory_order_relaxed);
  EXPECT_EQ(a.load(memory_order_relaxed), 1);
  a.store(2, memory_order_release);
  EXPECT_EQ(a.load(memory_order_acquire), 2);
  a.store(3, memory_order_seq_cst);
  EXPECT_EQ(a.load(memory_order_seq_cst), 3);
}

TEST(Atomic, Exchange) {
  Atomic<i32> a(10);
  i32 prev = a.exchange(99);
  EXPECT_EQ(prev, 10);
  EXPECT_EQ(a.load(), 99);
}

// ---- CAS -------------------------------------------------------------------

TEST(Atomic, CasStrongSucceeds) {
  Atomic<i32> a(5);
  i32 expected = 5;
  EXPECT_TRUE(a.compare_exchange_strong(expected, 6));
  EXPECT_EQ(a.load(), 6);
  EXPECT_EQ(expected, 5);
}

TEST(Atomic, CasStrongFails) {
  Atomic<i32> a(5);
  i32 expected = 999;
  EXPECT_FALSE(a.compare_exchange_strong(expected, 6));
  EXPECT_EQ(a.load(), 5);
  EXPECT_EQ(expected, 5);
}

TEST(Atomic, CasStrongTwoOrderOverload) {
  Atomic<i32> a(5);
  i32 expected = 5;
  EXPECT_TRUE(a.compare_exchange_strong(
      expected, 6, memory_order_acq_rel, memory_order_acquire));
  EXPECT_EQ(a.load(), 6);
}

TEST(Atomic, CasWeakEventuallySucceeds) {
  // weak CAS is allowed to spuriously fail. Loop until success.
  Atomic<i32> a(7);
  i32 expected = 7;
  while (!a.compare_exchange_weak(expected, 8)) {
    expected = 7;
  }
  EXPECT_EQ(a.load(), 8);
}

// ---- fetch arithmetic ------------------------------------------------------

TEST(Atomic, FetchAdd) {
  Atomic<i32> a(10);
  i32 prev = a.fetch_add(5);
  EXPECT_EQ(prev, 10);
  EXPECT_EQ(a.load(), 15);
}

TEST(Atomic, FetchSub) {
  Atomic<i32> a(20);
  i32 prev = a.fetch_sub(3);
  EXPECT_EQ(prev, 20);
  EXPECT_EQ(a.load(), 17);
}

TEST(Atomic, FetchAnd) {
  Atomic<u32> a(0xF0F0);
  u32 prev = a.fetch_and(0xFF00);
  EXPECT_EQ(prev, 0xF0F0u);
  EXPECT_EQ(a.load(), 0xF000u);
}

TEST(Atomic, FetchOr) {
  Atomic<u32> a(0xF000);
  u32 prev = a.fetch_or(0x00FF);
  EXPECT_EQ(prev, 0xF000u);
  EXPECT_EQ(a.load(), 0xF0FFu);
}

TEST(Atomic, FetchXor) {
  Atomic<u32> a(0xFF00);
  u32 prev = a.fetch_xor(0x00FF);
  EXPECT_EQ(prev, 0xFF00u);
  EXPECT_EQ(a.load(), 0xFFFFu);
}

TEST(Atomic, IncDecOperators) {
  Atomic<i32> a(5);
  EXPECT_EQ(++a, 6);
  EXPECT_EQ(a++, 6);
  EXPECT_EQ(a.load(), 7);
  EXPECT_EQ(--a, 6);
  EXPECT_EQ(a--, 6);
  EXPECT_EQ(a.load(), 5);
}

TEST(Atomic, CompoundAssign) {
  Atomic<i32> a(10);
  EXPECT_EQ(a += 5, 15);
  EXPECT_EQ(a -= 3, 12);
  Atomic<u32> b(0xF0F0);
  EXPECT_EQ(b &= 0xFF00, 0xF000u);
  EXPECT_EQ(b |= 0x000F, 0xF00Fu);
  EXPECT_EQ(b ^= 0xFF00, 0x0F0Fu);
}

TEST(Atomic, ImplicitLoadAndStoreOperators) {
  Atomic<i32> a(0);
  a = 42;
  i32 v = a;
  EXPECT_EQ(v, 42);
}

// ---- types -----------------------------------------------------------------

TEST(Atomic, BoolLoadStore) {
  Atomic<bool> a(false);
  EXPECT_FALSE(a.load());
  a.store(true);
  EXPECT_TRUE(a.load());
  // bool has no arithmetic path, just CAS.
  bool expected = true;
  EXPECT_TRUE(a.compare_exchange_strong(expected, false));
  EXPECT_FALSE(a.load());
}

TEST(Atomic, WideIntegers) {
  Atomic<u64> a(0);
  a.store(0xDEADBEEFCAFEBABEULL);
  EXPECT_EQ(a.load(), 0xDEADBEEFCAFEBABEULL);
  u64 prev = a.fetch_add(1);
  EXPECT_EQ(prev, 0xDEADBEEFCAFEBABEULL);
  EXPECT_EQ(a.load(), 0xDEADBEEFCAFEBABFULL);
}

TEST(Atomic, LockFreeness) {
  // On AMD64 and AArch64 u64 atomics are always lock-free.
  Atomic<u64> a;
  EXPECT_TRUE(a.is_lock_free());
  EXPECT_TRUE(Atomic<u64>::is_always_lock_free);
}

// ---- pointer specialization -----------------------------------------------

TEST(AtomicPointer, DefaultNull) {
  Atomic<int*> p;
  EXPECT_EQ(p.load(), nullptr);
}

TEST(AtomicPointer, LoadStore) {
  int x = 7;
  Atomic<int*> p(&x);
  EXPECT_EQ(p.load(), &x);
  int y = 9;
  p.store(&y);
  EXPECT_EQ(p.load(), &y);
}

TEST(AtomicPointer, Exchange) {
  int x = 1, y = 2;
  Atomic<int*> p(&x);
  EXPECT_EQ(p.exchange(&y), &x);
  EXPECT_EQ(p.load(), &y);
}

TEST(AtomicPointer, Cas) {
  int x = 1, y = 2;
  Atomic<int*> p(&x);
  int* expected = &x;
  EXPECT_TRUE(p.compare_exchange_strong(expected, &y));
  EXPECT_EQ(p.load(), &y);
  int* wrong = &x;
  EXPECT_FALSE(p.compare_exchange_strong(wrong, &y));
  EXPECT_EQ(wrong, &y);  // updated to current value
}

TEST(AtomicPointer, FetchAddScalesBySize) {
  int arr[4] = {10, 20, 30, 40};
  Atomic<int*> p(&arr[0]);
  int* prev = p.fetch_add(2);
  EXPECT_EQ(prev, &arr[0]);
  EXPECT_EQ(p.load(), &arr[2]);
  int* prev2 = p.fetch_sub(1);
  EXPECT_EQ(prev2, &arr[2]);
  EXPECT_EQ(p.load(), &arr[1]);
}

// ---- concurrent correctness -----------------------------------------------

TEST(Atomic, ConcurrentFetchAddCounter) {
  Atomic<u64> counter(0);
  constexpr int kThreads = 8;
  constexpr int kIterations = 100000;
  std::thread workers[kThreads];
  for (int i = 0; i < kThreads; ++i) {
    workers[i] = std::thread([&] {
      for (int k = 0; k < kIterations; ++k) {
        counter.fetch_add(1, memory_order_relaxed);
      }
    });
  }
  for (auto& t : workers) t.join();
  EXPECT_EQ(counter.load(), static_cast<u64>(kThreads) * kIterations);
}

TEST(Atomic, ConcurrentCasLinkedListPush) {
  // Lock-free Treiber-stack push racing many threads, ensure every pushed
  // node ends up linked exactly once and no pushes are lost.
  struct Node {
    int v;
    Node* next;
  };
  Atomic<Node*> head(nullptr);

  constexpr int kThreads = 8;
  constexpr int kPerThread = 10000;
  std::thread workers[kThreads];
  for (int t = 0; t < kThreads; ++t) {
    workers[t] = std::thread([&, t] {
      for (int i = 0; i < kPerThread; ++i) {
        Node* n = new Node{t * 1000000 + i, nullptr};
        Node* old_head = head.load(memory_order_relaxed);
        do {
          n->next = old_head;
        } while (!head.compare_exchange_weak(
            old_head, n, memory_order_release, memory_order_relaxed));
      }
    });
  }
  for (auto& w : workers) w.join();

  int count = 0;
  Node* it = head.load();
  while (it) {
    count++;
    Node* next = it->next;
    delete it;
    it = next;
  }
  EXPECT_EQ(count, kThreads * kPerThread);
}

TEST(AtomicThreadFence, Compiles) {
  atomic_thread_fence(memory_order_seq_cst);
  atomic_thread_fence(memory_order_acq_rel);
  atomic_thread_fence(memory_order_release);
  atomic_thread_fence(memory_order_acquire);
}

}  // namespace base
