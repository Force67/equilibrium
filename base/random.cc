// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <random>
#include <base/random.h>

namespace base {

template <typename T>
static T MersenneRandomIntegral(T start, T end) {
  auto mersenneRandom = std::mt19937(std::random_device{}());
  std::uniform_int_distribution<T> limits(start, end);

  return limits(mersenneRandom);
}

int RandomInt(int start, int end) {
  return MersenneRandomIntegral(start, end);
}

uint32_t RandomUint(uint32_t start, uint32_t end) {
  return MersenneRandomIntegral(start, end);
}
}  // namespace base