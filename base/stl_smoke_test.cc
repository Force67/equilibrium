// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Standalone smoke test for base::Atomic / Mutex / SharedMutex / Hash / Pair
// / move+forward+swap / Time / Random. Uses its own tiny EXPECT macros so we
// don't have to wire googletest into the standalone build path.
//
// Build on a posix host:
//
//   g++ -std=c++23 -DBASE_STL_SMOKE_STANDALONE -O2 \
//       -I<project-root> \
//       base/stl_smoke_test.cc base/threading/spinning_mutex.cc \
//       base/time/time_posix.cc base/random/random.cc base/random/random_posix.cc \
//       -pthread -o stl_smoke_test
//
// Without BASE_STL_SMOKE_STANDALONE the file is empty, so it is safe to leave
// in the source tree alongside the regular gtest-based atomic_test.cc.

#if defined(BASE_STL_SMOKE_STANDALONE)

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include <thread>

#include <base/arch.h>
#include <base/atomic.h>
#include <base/containers/pair.h>
#include <base/hashing/hash.h>
#include <base/memory/move.h>
#include <base/meta/traits.h>
#include <base/random/random.h>
#include <base/threading/lock_guard.h>
#include <base/threading/mutex.h>
#include <base/time/time.h>

namespace {

int g_tests_run = 0;
int g_tests_failed = 0;
const char* g_current = "";

#define RUN(name)                                               \
  do {                                                          \
    g_current = #name;                                          \
    g_tests_run++;                                              \
    printf("  [ RUN  ] %s\n", #name);                           \
    name();                                                     \
  } while (0)

#define EXPECT(cond)                                            \
  do {                                                          \
    if (!(cond)) {                                              \
      g_tests_failed++;                                         \
      printf("  [ FAIL ] %s: %s:%d  %s\n",                      \
             g_current, __FILE__, __LINE__, #cond);             \
      return;                                                   \
    }                                                           \
  } while (0)

#define EXPECT_EQ(a, b) EXPECT((a) == (b))
#define EXPECT_NE(a, b) EXPECT((a) != (b))
#define EXPECT_LT(a, b) EXPECT((a) < (b))
#define EXPECT_LE(a, b) EXPECT((a) <= (b))
#define EXPECT_GT(a, b) EXPECT((a) > (b))
#define EXPECT_GE(a, b) EXPECT((a) >= (b))
#define EXPECT_TRUE(c)  EXPECT(!!(c))
#define EXPECT_FALSE(c) EXPECT(!(c))

// ---------------------------------------------------------------------------
// base::Atomic
// ---------------------------------------------------------------------------

void Atomic_DefaultCtorZero() {
  base::Atomic<i32> a;
  EXPECT_EQ(a.load(), 0);
}
void Atomic_LoadStoreAllMemoryOrders() {
  base::Atomic<i32> a(0);
  a.store(1, base::memory_order_relaxed);
  EXPECT_EQ(a.load(base::memory_order_relaxed), 1);
  a.store(2, base::memory_order_release);
  EXPECT_EQ(a.load(base::memory_order_acquire), 2);
  a.store(3, base::memory_order_seq_cst);
  EXPECT_EQ(a.load(base::memory_order_seq_cst), 3);
}
void Atomic_Exchange() {
  base::Atomic<i32> a(10);
  EXPECT_EQ(a.exchange(99), 10);
  EXPECT_EQ(a.load(), 99);
}
void Atomic_CasStrong() {
  base::Atomic<i32> a(5);
  i32 e = 5;
  EXPECT_TRUE(a.compare_exchange_strong(e, 6));
  EXPECT_EQ(a.load(), 6);
  e = 999;
  EXPECT_FALSE(a.compare_exchange_strong(e, 7));
  EXPECT_EQ(e, 6);
  EXPECT_EQ(a.load(), 6);
}
void Atomic_CasStrongTwoOrders() {
  base::Atomic<i32> a(5);
  i32 e = 5;
  EXPECT_TRUE(a.compare_exchange_strong(
      e, 6, base::memory_order_acq_rel, base::memory_order_acquire));
  EXPECT_EQ(a.load(), 6);
}
void Atomic_CasWeak() {
  base::Atomic<i32> a(7);
  i32 e = 7;
  while (!a.compare_exchange_weak(e, 8)) e = 7;
  EXPECT_EQ(a.load(), 8);
}
void Atomic_FetchArithmetic() {
  base::Atomic<i32> a(10);
  EXPECT_EQ(a.fetch_add(5), 10);
  EXPECT_EQ(a.load(), 15);
  EXPECT_EQ(a.fetch_sub(3), 15);
  EXPECT_EQ(a.load(), 12);

  base::Atomic<u32> b(0xF0F0);
  EXPECT_EQ(b.fetch_and(0xFF00), 0xF0F0u);
  EXPECT_EQ(b.load(), 0xF000u);
  EXPECT_EQ(b.fetch_or(0x00FF), 0xF000u);
  EXPECT_EQ(b.load(), 0xF0FFu);
  EXPECT_EQ(b.fetch_xor(0x0F0F), 0xF0FFu);
  EXPECT_EQ(b.load(), 0xFFF0u);
}
void Atomic_IncDec() {
  base::Atomic<i32> a(5);
  EXPECT_EQ(++a, 6);
  EXPECT_EQ(a++, 6);
  EXPECT_EQ(a.load(), 7);
  EXPECT_EQ(--a, 6);
  EXPECT_EQ(a--, 6);
  EXPECT_EQ(a.load(), 5);
}
void Atomic_CompoundAssign() {
  base::Atomic<i32> a(10);
  EXPECT_EQ(a += 5, 15);
  EXPECT_EQ(a -= 3, 12);
  base::Atomic<u32> b(0xFF00);
  EXPECT_EQ(b &= 0xF0F0, 0xF000u);
  EXPECT_EQ(b |= 0x000F, 0xF00Fu);
  EXPECT_EQ(b ^= 0xFF00, 0x0F0Fu);
}
void Atomic_ImplicitLoadStore() {
  base::Atomic<i32> a(0);
  a = 42;
  i32 v = a;
  EXPECT_EQ(v, 42);
}
void Atomic_Bool() {
  base::Atomic<bool> a(false);
  EXPECT_FALSE(a.load());
  a.store(true);
  EXPECT_TRUE(a.load());
  bool e = true;
  EXPECT_TRUE(a.compare_exchange_strong(e, false));
  EXPECT_FALSE(a.load());
}
void Atomic_U64Wide() {
  base::Atomic<u64> a(0);
  a.store(0xDEADBEEFCAFEBABEULL);
  EXPECT_EQ(a.load(), 0xDEADBEEFCAFEBABEULL);
  EXPECT_EQ(a.fetch_add(1), 0xDEADBEEFCAFEBABEULL);
  EXPECT_EQ(a.load(), 0xDEADBEEFCAFEBABFULL);
}
void Atomic_LockFreeness() {
  base::Atomic<u64> a;
  EXPECT_TRUE(a.is_lock_free());
  EXPECT_TRUE(base::Atomic<u64>::is_always_lock_free);
}
void Atomic_PointerLoadStore() {
  int x = 7, y = 9;
  base::Atomic<int*> p(&x);
  EXPECT_EQ(p.load(), &x);
  p.store(&y);
  EXPECT_EQ(p.load(), &y);
  EXPECT_EQ(p.exchange(&x), &y);
  EXPECT_EQ(p.load(), &x);
}
void Atomic_PointerCas() {
  int x = 1, y = 2;
  base::Atomic<int*> p(&x);
  int* e = &x;
  EXPECT_TRUE(p.compare_exchange_strong(e, &y));
  EXPECT_EQ(p.load(), &y);
  int* wrong = &x;
  EXPECT_FALSE(p.compare_exchange_strong(wrong, &y));
  EXPECT_EQ(wrong, &y);
}
void Atomic_PointerFetchAddScalesBySize() {
  int arr[4] = {10, 20, 30, 40};
  base::Atomic<int*> p(&arr[0]);
  EXPECT_EQ(p.fetch_add(2), &arr[0]);
  EXPECT_EQ(p.load(), &arr[2]);
  EXPECT_EQ(p.fetch_sub(1), &arr[2]);
  EXPECT_EQ(p.load(), &arr[1]);
}
void Atomic_ConcurrentCounter() {
  base::Atomic<u64> counter(0);
  constexpr int kThreads = 8;
  constexpr int kIter = 100000;
  std::thread workers[kThreads];
  for (int i = 0; i < kThreads; ++i)
    workers[i] = std::thread([&] {
      for (int k = 0; k < kIter; ++k)
        counter.fetch_add(1, base::memory_order_relaxed);
    });
  for (auto& t : workers) t.join();
  EXPECT_EQ(counter.load(), static_cast<u64>(kThreads) * kIter);
}
void Atomic_ConcurrentTreiberStack() {
  struct Node { int v; Node* next; };
  base::Atomic<Node*> head(nullptr);
  constexpr int kThreads = 8;
  constexpr int kPer = 10000;
  std::thread workers[kThreads];
  for (int t = 0; t < kThreads; ++t)
    workers[t] = std::thread([&, t] {
      for (int i = 0; i < kPer; ++i) {
        Node* n = new Node{t * 1000000 + i, nullptr};
        Node* old_head = head.load(base::memory_order_relaxed);
        do { n->next = old_head; }
        while (!head.compare_exchange_weak(
            old_head, n, base::memory_order_release,
            base::memory_order_relaxed));
      }
    });
  for (auto& w : workers) w.join();
  int count = 0;
  Node* it = head.load();
  while (it) { count++; Node* n = it->next; delete it; it = n; }
  EXPECT_EQ(count, kThreads * kPer);
}
void Atomic_ThreadFenceCompiles() {
  base::atomic_thread_fence(base::memory_order_seq_cst);
  base::atomic_thread_fence(base::memory_order_acq_rel);
  base::atomic_thread_fence(base::memory_order_release);
  base::atomic_thread_fence(base::memory_order_acquire);
}

// ---------------------------------------------------------------------------
// base::Mutex / base::SharedMutex / lock guards
// ---------------------------------------------------------------------------

void Mutex_SingleThread() {
  base::Mutex m;
  m.lock();
  m.unlock();
  EXPECT_TRUE(m.try_lock());
  m.unlock();
}
void Mutex_LockGuardRaii() {
  base::Mutex m;
  {
    base::LockGuard<base::Mutex> lk(m);
    EXPECT_FALSE(m.try_lock());
  }
  EXPECT_TRUE(m.try_lock());
  m.unlock();
}
void Mutex_UniqueLockTryToLock() {
  base::Mutex m;
  m.lock();
  {
    base::UniqueLock<base::Mutex> lk(m, base::try_to_lock);
    EXPECT_FALSE(lk.owns_lock());
  }
  m.unlock();
  {
    base::UniqueLock<base::Mutex> lk(m, base::try_to_lock);
    EXPECT_TRUE(lk.owns_lock());
  }
  EXPECT_TRUE(m.try_lock());
  m.unlock();
}
void Mutex_ConcurrentIncrement() {
  base::Mutex m;
  int counter = 0;
  constexpr int kThreads = 8;
  constexpr int kIter = 20000;
  std::thread workers[kThreads];
  for (int i = 0; i < kThreads; ++i)
    workers[i] = std::thread([&] {
      for (int k = 0; k < kIter; ++k) {
        base::LockGuard<base::Mutex> lk(m);
        counter++;
      }
    });
  for (auto& w : workers) w.join();
  EXPECT_EQ(counter, kThreads * kIter);
}
void SharedMutex_ReadersAndWriters() {
  base::SharedMutex m;
  base::Atomic<int> readers(0);
  base::Atomic<int> max_readers(0);
  base::Atomic<int> writer_active(0);
  base::Atomic<int> writer_conflict(0);
  constexpr int kReaders = 6;
  constexpr int kWriters = 2;
  constexpr int kIter = 5000;

  auto reader = [&] {
    for (int i = 0; i < kIter; ++i) {
      base::SharedLockGuard<base::SharedMutex> lk(m);
      if (writer_active.load() != 0) writer_conflict.fetch_add(1);
      int r = readers.fetch_add(1) + 1;
      int cur = max_readers.load();
      while (r > cur && !max_readers.compare_exchange_weak(cur, r)) {}
      readers.fetch_sub(1);
    }
  };
  auto writer = [&] {
    for (int i = 0; i < kIter; ++i) {
      base::LockGuard<base::SharedMutex> lk(m);
      writer_active.store(1);
      if (readers.load() != 0) writer_conflict.fetch_add(1);
      writer_active.store(0);
    }
  };

  std::thread threads[kReaders + kWriters];
  for (int i = 0; i < kReaders; ++i) threads[i] = std::thread(reader);
  for (int i = 0; i < kWriters; ++i) threads[kReaders + i] = std::thread(writer);
  for (auto& t : threads) t.join();

  EXPECT_EQ(writer_conflict.load(), 0);
  EXPECT_GT(max_readers.load(), 1);  // at least two readers ran concurrently
}

// ---------------------------------------------------------------------------
// base::Hash / Pair / MakePair / move/forward/swap / traits
// ---------------------------------------------------------------------------

void Hash_IntDeterministic() {
  base::Hash<int> h;
  EXPECT_EQ(h(42), h(42));
  EXPECT_NE(h(42), h(43));
}
void Hash_IntAvoidsIdentity() {
  base::Hash<int> h;
  EXPECT_NE(h(0), 0u);
  EXPECT_NE(h(1), 1u);
  EXPECT_NE(h(2), 2u);
}
void Hash_Pointer() {
  int x = 0;
  int y = 0;
  base::Hash<int*> h;
  EXPECT_EQ(h(&x), h(&x));
  EXPECT_NE(h(&x), h(&y));
}
void Hash_Enum() {
  enum class E { A, B, C };
  base::Hash<E> h;
  EXPECT_EQ(h(E::A), h(E::A));
  EXPECT_NE(h(E::A), h(E::B));
}
void Hash_Bytes() {
  const char* a = "hello";
  const char* b = "hella";
  EXPECT_NE(base::HashBytes(a, 5), base::HashBytes(b, 5));
  EXPECT_EQ(base::HashBytes(a, 5), base::HashBytes(a, 5));
}
void Pair_ValueAccess() {
  base::Pair<int, const char*> p{7, "x"};
  EXPECT_EQ(p.first, 7);
  EXPECT_EQ(strcmp(p.second, "x"), 0);
}
void Pair_MakePairForwards() {
  auto p = base::MakePair(1, 2.5);
  EXPECT_EQ(p.first, 1);
  EXPECT_EQ(p.second, 2.5);
}
void Move_DoesMove() {
  struct S {
    int v;
    S(int x) : v(x) {}
    S(S&& o) noexcept : v(o.v) { o.v = -1; }
    S(const S&) = delete;
  };
  S a(10);
  S b(base::move(a));
  EXPECT_EQ(b.v, 10);
  EXPECT_EQ(a.v, -1);
}
void Forward_PreservesRefness() {
  struct Sink {
    static const char* f(int&)       { return "lvalue"; }
    static const char* f(const int&) { return "const"; }
    static const char* f(int&&)      { return "rvalue"; }
  };
  auto call = []<typename T>(T&& x) {
    return Sink::f(base::forward<T>(x));
  };
  int n = 1;
  EXPECT_EQ(strcmp(call(n), "lvalue"), 0);
  EXPECT_EQ(strcmp(call(5), "rvalue"), 0);
}
void Swap_WorksOnPod() {
  int a = 1, b = 2;
  base::swap(a, b);
  EXPECT_EQ(a, 2);
  EXPECT_EQ(b, 1);
}
void Traits_Basics() {
  EXPECT_TRUE((base::is_same_v<int, int>));
  EXPECT_FALSE((base::is_same_v<int, long>));
  EXPECT_TRUE(base::is_integral_v<u64>);
  EXPECT_FALSE(base::is_integral_v<double>);
  EXPECT_TRUE(base::is_signed_v<i32>);
  EXPECT_FALSE(base::is_signed_v<u32>);
}

// ---------------------------------------------------------------------------
// base::Time / TickClock
// ---------------------------------------------------------------------------

void Time_UnixTimeStampMonotonicEnough() {
  i64 a = base::GetUnixTimeStamp();
  i64 b = base::GetUnixTimeStamp();
  EXPECT_LE(a, b);
  EXPECT_GT(a, 1700000000LL);
}
void Time_UnixMillisecondsGreaterThanSeconds() {
  i64 s = base::GetUnixTimeStamp();
  i64 ms = base::GetUnixTimeMilliseconds();
  EXPECT_LT((s - 1) * 1000LL, ms);  // ms >= s*1000 within drift
}
void Time_TickClockMonotonic() {
  i64 t0 = base::TickClock::NowNs();
  for (volatile int i = 0; i < 100000; ++i) {}
  i64 t1 = base::TickClock::NowNs();
  EXPECT_GT(t1, t0);
}

// ---------------------------------------------------------------------------
// base::Random (xoshiro-backed)
// ---------------------------------------------------------------------------

void Random_IntRangeRespected() {
  for (int i = 0; i < 1000; ++i) {
    i32 v = base::RandomInt(10, 20);
    EXPECT_GE(v, 10);
    EXPECT_LE(v, 20);
  }
}
void Random_UintCoversRange() {
  // 1000 draws from [0, 1023] should hit "most" of the range.
  bool seen[1024]{};
  for (int i = 0; i < 10000; ++i) {
    u32 v = base::RandomUint(0u, 1023u);
    EXPECT_LT(v, 1024u);
    seen[v] = true;
  }
  int hits = 0;
  for (bool b : seen) hits += b ? 1 : 0;
  // 10000 draws in 1024 slots: expected coverage ~all slots. Require >=900.
  EXPECT_GT(hits, 900);
}
void Random_SeedNonZero() {
  // True random seed must not be zero (would be a kernel-CSPRNG failure).
  u64 seed = base::SourceTrueRandomSeed();
  EXPECT_NE(seed, 0ull);
}
void Random_SeedChangesBetweenCalls() {
  u64 a = base::SourceTrueRandomSeed();
  u64 b = base::SourceTrueRandomSeed();
  // ~1-in-2^64 chance of collision.
  EXPECT_NE(a, b);
}

}  // namespace

int main() {
  printf("=== base STL-free smoke tests ===\n");

  RUN(Atomic_DefaultCtorZero);
  RUN(Atomic_LoadStoreAllMemoryOrders);
  RUN(Atomic_Exchange);
  RUN(Atomic_CasStrong);
  RUN(Atomic_CasStrongTwoOrders);
  RUN(Atomic_CasWeak);
  RUN(Atomic_FetchArithmetic);
  RUN(Atomic_IncDec);
  RUN(Atomic_CompoundAssign);
  RUN(Atomic_ImplicitLoadStore);
  RUN(Atomic_Bool);
  RUN(Atomic_U64Wide);
  RUN(Atomic_LockFreeness);
  RUN(Atomic_PointerLoadStore);
  RUN(Atomic_PointerCas);
  RUN(Atomic_PointerFetchAddScalesBySize);
  RUN(Atomic_ConcurrentCounter);
  RUN(Atomic_ConcurrentTreiberStack);
  RUN(Atomic_ThreadFenceCompiles);

  RUN(Mutex_SingleThread);
  RUN(Mutex_LockGuardRaii);
  RUN(Mutex_UniqueLockTryToLock);
  RUN(Mutex_ConcurrentIncrement);
  RUN(SharedMutex_ReadersAndWriters);

  RUN(Hash_IntDeterministic);
  RUN(Hash_IntAvoidsIdentity);
  RUN(Hash_Pointer);
  RUN(Hash_Enum);
  RUN(Hash_Bytes);
  RUN(Pair_ValueAccess);
  RUN(Pair_MakePairForwards);
  RUN(Move_DoesMove);
  RUN(Forward_PreservesRefness);
  RUN(Swap_WorksOnPod);
  RUN(Traits_Basics);

  RUN(Time_UnixTimeStampMonotonicEnough);
  RUN(Time_UnixMillisecondsGreaterThanSeconds);
  RUN(Time_TickClockMonotonic);

  RUN(Random_IntRangeRespected);
  RUN(Random_UintCoversRange);
  RUN(Random_SeedNonZero);
  RUN(Random_SeedChangesBetweenCalls);

  printf("\n=== %d/%d passed, %d failed ===\n",
         g_tests_run - g_tests_failed, g_tests_run, g_tests_failed);
  return g_tests_failed == 0 ? 0 : 1;
}

#endif  // BASE_STL_SMOKE_STANDALONE
