// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// xoshiro256** (https://prng.di.unimi.it/) seeded from the kernel CSPRNG
// via SourceTrueRandomSeed. Passes BigCrush; mt19937 does not.

#include "random.h"
#include "base/external/xoshiro256ss/xoshiro256ss.h"

namespace base {

namespace {

// Unbiased bounded draw: reject values landing in the [limit, U_MAX] tail
// so every value in [lo, hi] is equally likely.
template <typename T>
T BoundedDraw(xoshiro256ss& rng, T lo, T hi) {
  using U = unsigned long long;
  const U range = static_cast<U>(hi) - static_cast<U>(lo) + 1u;
  if (range == 0)
    return static_cast<T>(rng());

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
