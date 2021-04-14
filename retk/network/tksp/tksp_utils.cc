// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <network/tksp/tksp_utils.h>

namespace network::tksp {

uint32_t GetRandomSeed(uint32_t reserve) {
  auto mersenneRandom = std::mt19937(std::random_device{}());
  // give us a random value between 0 and limit
  // some of those upper values are reserved for the system.
  std::uniform_int_distribution<uint32_t> limits(0, UINT_MAX - reserve);

  return limits(mersenneRandom);
}
}