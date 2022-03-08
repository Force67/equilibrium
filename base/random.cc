// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <random>
#include "base/random.h"
#include "base/external/xoshiro256ss/xoshiro256ss.h"

namespace base {

template <typename T>
static T XORandomIntegral(T start, T end) {
  xoshiro256ss rng(std::random_device{}());
  std::uniform_int_distribution<T> limits(start, end);

  return limits(rng);
}

i32 RandomInt(i32 start, i32 end) {
  return XORandomIntegral(start, end);
}

u32 RandomUint(u32 start, u32 end) {
  return XORandomIntegral(start, end);
}
}  // namespace base