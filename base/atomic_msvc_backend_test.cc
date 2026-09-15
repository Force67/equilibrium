// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Runs base/atomic.h's MSVC _Interlocked* backend on whatever host is
// building, by standing its intrinsics up on the __atomic_* builtins (see
// atomic_msvc_intrin_shim.h). Without this the backend would only ever be
// compiled by the one compiler nobody here builds with, and would ship
// having never executed.
//
// This is its own binary, not part of base_unittests, on purpose: forcing
// the MSVC backend on gives base::Atomic a second, different definition, and
// linking both into one image would be an ODR violation. It also brings its
// own EXPECT macros rather than googletest, matching stl_smoke_test.cc.

#define BASE_ATOMIC_MSVC_INTRINSICS 1
#define BASE_ATOMIC_INTRIN_HEADER <base/atomic_msvc_intrin_shim.h>

#include <base/atomic.h>

#include <pthread.h>
#include <stdio.h>

static int g_fail = 0;
#define CHECK(cond)                                                     \
  do {                                                                  \
    if (!(cond)) {                                                      \
      printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);            \
      g_fail++;                                                         \
    }                                                                   \
  } while (0)

template <class T>
static void ExerciseInteger(const char* name) {
  printf("  -- %s (%zu bytes)\n", name, sizeof(T));
  base::Atomic<T> a{};
  CHECK(a.load() == T{0});

  a.store(static_cast<T>(7));
  CHECK(a.load() == static_cast<T>(7));

  // Every order must round-trip, since load/store are the only operations
  // that branch on it.
  const base::memory_order kLoadOrders[] = {base::memory_order_relaxed,
                                            base::memory_order_acquire,
                                            base::memory_order_seq_cst};
  const base::memory_order kStoreOrders[] = {base::memory_order_relaxed,
                                             base::memory_order_release,
                                             base::memory_order_seq_cst};
  for (int i = 0; i < 3; i++) {
    a.store(static_cast<T>(21), kStoreOrders[i]);
    CHECK(a.load(kLoadOrders[i]) == static_cast<T>(21));
  }

  CHECK(a.exchange(static_cast<T>(9)) == static_cast<T>(21));
  CHECK(a.load() == static_cast<T>(9));

  // CAS success leaves expected alone; failure must write back what was read.
  T expected = static_cast<T>(9);
  CHECK(a.compare_exchange_strong(expected, static_cast<T>(11)));
  CHECK(a.load() == static_cast<T>(11));
  expected = static_cast<T>(99);
  CHECK(!a.compare_exchange_strong(expected, static_cast<T>(13)));
  CHECK(expected == static_cast<T>(11));
  CHECK(a.load() == static_cast<T>(11));

  a.store(static_cast<T>(10));
  CHECK(a.fetch_add(static_cast<T>(5)) == static_cast<T>(10));
  CHECK(a.load() == static_cast<T>(15));
  // fetch_sub goes through xadd of a negated value; this is the case that
  // catches a sign or width mistake in that negation.
  CHECK(a.fetch_sub(static_cast<T>(6)) == static_cast<T>(15));
  CHECK(a.load() == static_cast<T>(9));

  a.store(static_cast<T>(0b1100));
  CHECK(a.fetch_and(static_cast<T>(0b1010)) == static_cast<T>(0b1100));
  CHECK(a.load() == static_cast<T>(0b1000));
  CHECK(a.fetch_or(static_cast<T>(0b0011)) == static_cast<T>(0b1000));
  CHECK(a.load() == static_cast<T>(0b1011));
  CHECK(a.fetch_xor(static_cast<T>(0b1111)) == static_cast<T>(0b1011));
  CHECK(a.load() == static_cast<T>(0b0100));

  a.store(static_cast<T>(5));
  CHECK(++a == static_cast<T>(6));
  CHECK(a++ == static_cast<T>(6));
  CHECK(a.load() == static_cast<T>(7));
  CHECK(--a == static_cast<T>(6));
  CHECK(a-- == static_cast<T>(6));
  CHECK(a.load() == static_cast<T>(5));
  CHECK((a += static_cast<T>(3)) == static_cast<T>(8));
  CHECK((a -= static_cast<T>(2)) == static_cast<T>(6));

  CHECK(a.is_lock_free());
  static_assert(base::Atomic<T>::is_always_lock_free);
}

int main() {
  printf("MSVC atomic backend, driven through shimmed intrinsics\n");
  ExerciseInteger<u8>("u8");
  ExerciseInteger<i8>("i8");
  ExerciseInteger<u16>("u16");
  ExerciseInteger<i16>("i16");
  ExerciseInteger<u32>("u32");
  ExerciseInteger<i32>("i32");
  ExerciseInteger<u64>("u64");
  ExerciseInteger<i64>("i64");

  printf("  -- bool\n");
  base::Atomic<bool> flag{false};
  CHECK(flag.load() == false);
  flag.store(true);
  CHECK(flag.load() == true);
  CHECK(flag.exchange(false) == true);

  printf("  -- pointers\n");
  int values[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  base::Atomic<int*> p{&values[0]};
  CHECK(p.load() == &values[0]);
  // fetch_add must scale by sizeof(int), not add raw bytes.
  CHECK(p.fetch_add(3) == &values[0]);
  CHECK(p.load() == &values[3]);
  CHECK(p.fetch_sub(2) == &values[3]);
  CHECK(p.load() == &values[1]);
  CHECK(p.exchange(&values[7]) == &values[1]);
  int* expected_p = &values[7];
  CHECK(p.compare_exchange_strong(expected_p, &values[0]));
  CHECK(p.load() == &values[0]);
  expected_p = &values[5];
  CHECK(!p.compare_exchange_strong(expected_p, &values[2]));
  CHECK(expected_p == &values[0]);

  printf("  -- non-integer payload\n");
  struct Pair { short a; short b; };
  base::Atomic<Pair> pair{Pair{1, 2}};
  CHECK(pair.load().a == 1 && pair.load().b == 2);
  pair.store(Pair{3, 4});
  CHECK(pair.load().a == 3 && pair.load().b == 4);
  Pair expected_pair{3, 4};
  CHECK(pair.compare_exchange_strong(expected_pair, Pair{5, 6}));
  CHECK(pair.load().a == 5 && pair.load().b == 6);

  printf("  -- contention\n");
  {
    // Single-threaded checks cannot tell an atomic increment from a plain
    // one. This can: four threads racing on one counter and one CAS-guarded
    // word, where a lost update shows up as a short count.
    constexpr int kThreads = 4;
    constexpr int kPerThread = 50000;
    static base::Atomic<u64> counter{0};
    static base::Atomic<u32> cas_counter{0};

    auto worker = [](void*) -> void* {
      for (int i = 0; i < kPerThread; i++) {
        counter.fetch_add(1);
        u32 seen = cas_counter.load(base::memory_order_relaxed);
        while (!cas_counter.compare_exchange_weak(
            seen, seen + 1, base::memory_order_acq_rel,
            base::memory_order_relaxed)) {
        }
      }
      return nullptr;
    };

    pthread_t threads[kThreads];
    for (int t = 0; t < kThreads; t++)
      ::pthread_create(&threads[t], nullptr, worker, nullptr);
    for (int t = 0; t < kThreads; t++)
      ::pthread_join(threads[t], nullptr);

    CHECK(counter.load() == u64{kThreads} * kPerThread);
    CHECK(cas_counter.load() == u32{kThreads} * kPerThread);
  }

  base::atomic_thread_fence(base::memory_order_seq_cst);
  base::atomic_thread_fence(base::memory_order_acquire);
  base::atomic_thread_fence(base::memory_order_relaxed);

  printf(g_fail ? "\n%d CHECK(s) FAILED\n" : "\nall checks passed\n", g_fail);
  return g_fail != 0;
}
