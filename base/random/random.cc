// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// All generators are backed by xoshiro256**, see https://prng.di.unimi.it/.
//
// Quality vs. mt19937 (the canonical Mersenne Twister):
//   - xoshiro256** passes BigCrush (TestU01). mt19937 fails linearity tests.
//   - Period 2^256-1 vs 2^19937-1: both are astronomically large; 2^256-1
//     suffices for any conceivable workload.
//   - State size 32 B vs 2.5 KiB; speed ~2-3x faster than MT.
//   - Seed source is the kernel CSPRNG (/dev/urandom / CryptGenRandom) via
//     SourceTrueRandomSeed(), which is strictly stronger than the typical
//     std::random_device{}() seed commonly used to seed MT.

#include "random.h"
#include "base/external/xoshiro256ss/xoshiro256ss.h"

namespace base {

namespace {

// Unbiased bounded integer draw. Classic rejection-sampling pattern: ignore
// any value that falls in the "uneven remainder" tail so [lo, hi] stays
// perfectly uniform.
template <typename T>
T BoundedDraw(xoshiro256ss& rng, T lo, T hi) {
  using U = unsigned long long;
  const U range = static_cast<U>(hi) - static_cast<U>(lo) + 1u;
  if (range == 0)
    return static_cast<T>(rng());  // caller passed full range

  // Largest multiple of `range` that fits in a u64.
  const U limit = U(-1) - (U(-1) % range);
  U x;
  do {
    x = rng();
  } while (x >= limit);
  return static_cast<T>(lo + static_cast<T>(x % range));
}

template <typename T>
T XORandomIntegral(T start, T end) {
  xoshiro256ss rng(base::SourceTrueRandomSeed());
  return BoundedDraw(rng, start, end);
}

}  // namespace

i32 RandomInt(i32 start, i32 end) {
  return XORandomIntegral(start, end);
}

u32 RandomUint(u32 start, u32 end) {
  return XORandomIntegral(start, end);
}
}  // namespace base
